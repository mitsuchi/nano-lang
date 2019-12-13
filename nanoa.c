#include <stdio.h>
#include <stdbool.h>
#include <string.h>
#include <elf.h>

int main() {
  char buffer[1024];
  char *string = NULL;
  bool string_found = false;

  while (fgets(buffer, 1024, stdin) != NULL) {
    char *p = buffer;

    while (*p) {
      if (*p == '"' && string == NULL) {
        p++;
        string = p;
      }
      if (*p == '"' && string != NULL ) {
        *p = '\0';
	string_found = true;
        break;
      }
      p++;
    }
    if (string_found) {
      break;
    }
  }
  if (string == NULL) {
    return 1;
  }
  int string_len = strlen(string);
  int text_section_offset = 64;
  int text_section_size = string_len + 1 + 23;
  int text_section_padding = 8 - (text_section_offset + text_section_size) % 8;
  int symtab_section_offset = text_section_offset + text_section_size + text_section_padding;
  
  // ELF header
  Elf64_Ehdr hdr;
  hdr.e_ident[0] = ELFMAG0;
  hdr.e_ident[1] = ELFMAG1;
  hdr.e_ident[2] = ELFMAG2;
  hdr.e_ident[3] = ELFMAG3;
  hdr.e_ident[4] = ELFCLASS64;
  hdr.e_ident[5] = ELFDATA2LSB;
  hdr.e_ident[6] = EV_CURRENT;
  hdr.e_ident[7] = ELFOSABI_SYSV;
  hdr.e_ident[8] = 0;
  for( int i=9; i<16; i++ ) {
    hdr.e_ident[i] = 0;
  }
  hdr.e_type = ET_REL;
  hdr.e_machine = EM_X86_64;
  hdr.e_version = EV_CURRENT;
  hdr.e_entry = 0;
  hdr.e_phoff = 0;
  hdr.e_shoff = symtab_section_offset + 288; 
  hdr.e_flags = 0;
  hdr.e_ehsize = 0x40;
  hdr.e_phentsize = 0;
  hdr.e_phnum = 0;
  hdr.e_shentsize = 0x40;
  hdr.e_shnum = 8;
  hdr.e_shstrndx = 7;
  fwrite(&hdr, sizeof(hdr), 1, stdout);

  // .text section
  int text_addr = 255 - string_len - 11;
  fwrite(string, string_len+1, 1, stdout);

  char program[] = 
    //{ 0x55, 0x48, 0x89, 0xe5, 0x48, 0x8d, 0x3d, 0xe8,
    { 0x55, 0x48, 0x89, 0xe5, 0x48, 0x8d, 0x3d, text_addr,
      0xff, 0xff, 0xff, 0xe8, 0x00, 0x00, 0x00, 0x00,
      0xb8, 0x00, 0x00, 0x00, 0x00, 0x5d, 0xc3 };
  fwrite(&program, sizeof(program), 1, stdout);
  char padding[text_section_padding];
  for (int i=0; i<text_section_padding; i++) {
    padding[i] = 0x0;
  }
  fwrite(&padding, sizeof(padding), 1, stdout);
  
  Elf64_Sym sym[7];
  sym[0].st_name = 0;
  sym[0].st_info = 0;
  sym[0].st_other = 0;
  sym[0].st_shndx = 0;
  sym[0].st_value = 0;
  sym[0].st_size = 0;

  sym[1].st_name = 0;
  sym[1].st_info = ELF64_ST_INFO(0, 3);
  sym[1].st_other = 0;
  sym[1].st_shndx = 1;
  sym[1].st_value = 0;
  sym[1].st_size = 0;
  
  sym[2].st_name = 0;
  sym[2].st_info = ELF64_ST_INFO(0, 3);
  sym[2].st_other = 0;
  sym[2].st_shndx = 3;
  sym[2].st_value = 0;
  sym[2].st_size = 0;

  sym[3].st_name = 0;
  sym[3].st_info = ELF64_ST_INFO(0, 3);
  sym[3].st_other = 0;
  sym[3].st_shndx = 4;
  sym[3].st_value = 0;
  sym[3].st_size = 0;

  // main
  sym[4].st_name = 1;
  sym[4].st_info = ELF64_ST_INFO(1, 0);
  sym[4].st_other = 0;
  sym[4].st_shndx = 1;
  sym[4].st_value = string_len + 1;
  sym[4].st_size = 0;

  sym[5].st_name = 6;
  sym[5].st_info = ELF64_ST_INFO(1, 0);
  sym[5].st_other = 0;
  sym[5].st_shndx = 0;
  sym[5].st_value = 0;
  sym[5].st_size = 0;

  sym[6].st_name = 0xb;
  sym[6].st_info = ELF64_ST_INFO(1, 0);
  sym[6].st_other = 0;
  sym[6].st_shndx = 0;
  sym[6].st_value = 0;
  sym[6].st_size = 0;
  fwrite(&sym, sizeof(sym), 1, stdout);

  char strtab[] = "\0main\0puts\0_GLOBAL_OFFSET_TABLE_";
  fwrite(&strtab, sizeof(strtab), 1, stdout);
  char strtab_pad[] = "\0\0\0\0\0\0";
  fwrite(&strtab_pad, sizeof(strtab_pad), 1, stdout);

  Elf64_Rela rela_text;
  rela_text.r_offset = string_len + 13;
  rela_text.r_info = ELF64_R_INFO(5,4);
  rela_text.r_addend = 0xfffffffffffffffc;
  fwrite(&rela_text, sizeof(rela_text), 1, stdout);

  char shstrtab[] = "\0.symtab\0.strtab\0.shstrtab\0.rela.text\0.data\0.bss";
  fwrite(&shstrtab, sizeof(shstrtab), 1, stdout);
  char shstrtab_pad[] = "\0\0\0\0\0\0";
  fwrite(&shstrtab_pad, sizeof(shstrtab_pad), 1, stdout);

  Elf64_Shdr sh[8];

  // section 0
  sh[0].sh_name = 0;
  sh[0].sh_type = 0;
  sh[0].sh_flags = 0;
  sh[0].sh_addr = 0;
  sh[0].sh_offset = 0;
  sh[0].sh_size = 0;
  sh[0].sh_link = 0;
  sh[0].sh_info = 0;
  sh[0].sh_addralign = 0;
  sh[0].sh_entsize = 0;

  // .text section
  sh[1].sh_name = 0x20;
  sh[1].sh_type = 1;
  sh[1].sh_flags = 6;
  sh[1].sh_addr = 0;
  sh[1].sh_offset = text_section_offset; 
  sh[1].sh_size = text_section_size; 
  sh[1].sh_link = 0;
  sh[1].sh_info = 0;
  sh[1].sh_addralign = 1;
  sh[1].sh_entsize = 0;

  // .real.text section
  sh[2].sh_name = 0x1b;
  sh[2].sh_type = 4;
  sh[2].sh_flags = 0x40;
  sh[2].sh_addr = 0;
  sh[2].sh_offset = symtab_section_offset + 208; 
  sh[2].sh_size = 0x18;
  sh[2].sh_link = 5;
  sh[2].sh_info = 1;
  sh[2].sh_addralign = 8;
  sh[2].sh_entsize = 0x18;

  // .data section
  sh[3].sh_name = 0x26;
  sh[3].sh_type = 1;
  sh[3].sh_flags = 0x03;
  sh[3].sh_addr = 0;
  sh[3].sh_offset = text_section_offset + text_section_size; 
  sh[3].sh_size = 0;
  sh[3].sh_link = 0;
  sh[3].sh_info = 0;
  sh[3].sh_addralign = 1;
  sh[3].sh_entsize = 0;

  // .bss section
  sh[4].sh_name = 0x2c;
  sh[4].sh_type = 8;
  sh[4].sh_flags = 0x03;
  sh[4].sh_addr = 0;
  sh[4].sh_offset = text_section_offset + text_section_size; 
  sh[4].sh_size = 0;
  sh[4].sh_link = 0;
  sh[4].sh_info = 0;
  sh[4].sh_addralign = 1;
  sh[4].sh_entsize = 0;

  // .symtab section
  sh[5].sh_name = 0x01;
  sh[5].sh_type = 2;
  sh[5].sh_flags = 0x00;
  sh[5].sh_addr = 0;
  sh[5].sh_offset = symtab_section_offset; 
  sh[5].sh_size = 0xa8; 
  sh[5].sh_link = 0x06;
  sh[5].sh_info = 0x04;
  sh[5].sh_addralign = 8;
  sh[5].sh_entsize = 0x18;

  // .strtab section
  sh[6].sh_name = 0x09;
  sh[6].sh_type = 3;
  sh[6].sh_flags = 0x00;
  sh[6].sh_addr = 0;
  sh[6].sh_offset = symtab_section_offset + 168; 
  sh[6].sh_size = 0x21;
  sh[6].sh_link = 0x00;
  sh[6].sh_info = 0x00;
  sh[6].sh_addralign = 1;
  sh[6].sh_entsize = 0x0;

  // .shstrtab section
  sh[7].sh_name = 0x11;
  sh[7].sh_type = 3;
  sh[7].sh_flags = 0x00;
  sh[7].sh_addr = 0;
  sh[7].sh_offset = symtab_section_offset + 232; 
  sh[7].sh_size = 0x31;
  sh[7].sh_link = 0x00;
  sh[7].sh_info = 0x00;
  sh[7].sh_addralign = 1;
  sh[7].sh_entsize = 0x0;

  fwrite(&sh, sizeof(sh), 1, stdout);
}

