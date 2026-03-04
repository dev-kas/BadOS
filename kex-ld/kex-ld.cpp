#include <iostream>
#include <vector>
#include <fstream>
#include <cstring>
#include <openssl/sha.h>
#include <elf.h>
#include "kex.h"

void write_padding(std::ostream& out, uint64_t target) {
    uint64_t current = out.tellp();
    if (current < target) {
        std::vector<char> pad(target - current, 0);
        out.write(pad.data(), pad.size());
    }
}

int main(int argc, char** argv) {
    if (argc < 3) {
        std::cerr << "Usage: kex-ld <input.elf> <output.kex>" << std::endl;
        return 1;
    }

    std::ifstream elf_f(argv[1], std::ios::binary);
    if (!elf_f) return 1;

    // read ELF magic to determine 32 or 64 bit
    uint8_t e_ident[EI_NIDENT];
    elf_f.read((char*)e_ident, EI_NIDENT);
    elf_f.seekg(0);

    uint64_t entry_point = 0;
    uint16_t machine = 0;
    struct TempSeg { uint32_t flags; uint64_t vaddr, memsz, off, filesz; };
    std::vector<TempSeg> elf_segments;

    if (e_ident[EI_CLASS] == ELFCLASS32) {
        Elf32_Ehdr ehdr;
        elf_f.read((char*)&ehdr, sizeof(ehdr));
        entry_point = ehdr.e_entry;
        machine = ehdr.e_machine;
        
        elf_f.seekg(ehdr.e_phoff);
        for(int i=0; i<ehdr.e_phnum; i++) {
            Elf32_Phdr ph;
            elf_f.read((char*)&ph, sizeof(ph));
            if(ph.p_type == PT_LOAD) 
                elf_segments.push_back({ph.p_flags, ph.p_vaddr, ph.p_memsz, (uint64_t)ph.p_offset, ph.p_filesz});
        }
    } else {
        Elf64_Ehdr ehdr;
        elf_f.read((char*)&ehdr, sizeof(ehdr));
        entry_point = ehdr.e_entry;
        machine = ehdr.e_machine;

        elf_f.seekg(ehdr.e_phoff);
        for(int i=0; i<ehdr.e_phnum; i++) {
            Elf64_Phdr ph;
            elf_f.read((char*)&ph, sizeof(ph));
            if(ph.p_type == PT_LOAD) 
                elf_segments.push_back({ph.p_flags, ph.p_vaddr, ph.p_memsz, ph.p_offset, ph.p_filesz});
        }
    }

    // init KEX header
    kex_header hdr = {0};
    memcpy(hdr.magic, KEX_MAGIC, 4);
    hdr.version_major = 1;
    hdr.version_minor = 0;
    // map ELF machine to KEX arch
    if (machine == EM_X86_64) hdr.arch = KEX_ARCH_X86_64;
    else if (machine == EM_386) hdr.arch = 0x8001; // private ID for x86_32
    else hdr.arch = 0xFFFF;
    
    hdr.command_count = elf_segments.size() + 1; // segments + integrity
    hdr.required_page_size = 4096;
    hdr.entry_rva = entry_point;
    hdr.cmd_offset = sizeof(kex_header);
    hdr.cmd_size = (elf_segments.size() * sizeof(kex_segment_cmd)) + sizeof(kex_integrity_cmd);

    // prepare commands
    std::vector<kex_segment_cmd> kex_segs;
    uint64_t current_file_pos = (hdr.cmd_offset + hdr.cmd_size + 4095) & ~4095;

    for (size_t i = 0; i < elf_segments.size(); i++) {
        kex_segment_cmd ks = {0};
        ks.head.type = KEX_CMD_SEGMENT;
        ks.head.size = sizeof(kex_segment_cmd);
        ks.segment_index = i;
        // map ELF flags to KEX (R=1, W=2, X=4)
        if (elf_segments[i].flags & PF_R) ks.flags |= 0x1;
        if (elf_segments[i].flags & PF_W) ks.flags |= 0x2;
        if (elf_segments[i].flags & PF_X) ks.flags |= 0x4;
        
        ks.rva = elf_segments[i].vaddr;
        ks.mem_size = elf_segments[i].memsz;
        ks.file_size = elf_segments[i].filesz;
        ks.alignment = 4096;
        
        if (ks.file_size > 0) {
            ks.file_offset = current_file_pos;
            current_file_pos = (current_file_pos + ks.file_size + 4095) & ~4095;
        }
        kex_segs.push_back(ks);
    }

    // write file
    std::ofstream out(argv[2], std::ios::binary);
    out.write((char*)&hdr, sizeof(hdr));
    for (auto& s : kex_segs) out.write((char*)&s, sizeof(s));

    // integrity placeholder
    uint64_t integrity_pos = out.tellp();
    kex_integrity_cmd icmd = {0};
    icmd.head.type = KEX_CMD_INTEGRITY;
    icmd.head.size = sizeof(icmd);
    out.write((char*)&icmd, sizeof(icmd));

    // write segment data
    for (size_t i = 0; i < elf_segments.size(); i++) {
        if (kex_segs[i].file_size > 0) {
            write_padding(out, kex_segs[i].file_offset);
            std::vector<char> buffer(kex_segs[i].file_size);
            elf_f.seekg(elf_segments[i].off);
            elf_f.read(buffer.data(), kex_segs[i].file_size);
            out.write(buffer.data(), kex_segs[i].file_size);
        }
    }
    out.flush();

    // calculate integrity hash (scope: [0, integrity_cmd_offset))
    out.close();
    std::ifstream rescan(argv[2], std::ios::binary);
    std::vector<unsigned char> full_file((std::istreambuf_iterator<char>(rescan)), std::istreambuf_iterator<char>());
    SHA256(full_file.data(), integrity_pos, icmd.hash);

    // patch integrity
    std::fstream patch(argv[2], std::ios::binary | std::ios::in | std::ios::out);
    patch.seekp(integrity_pos);
    patch.write((char*)&icmd, sizeof(icmd));
    
    std::cout << "Built " << argv[2] << " (" << full_file.size() << " bytes)" << std::endl;
    return 0;
}
