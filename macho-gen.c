#include <mach-o/loader.h>
#include <string.h>
#include <stdio.h>

int main() {

	struct mach_header_64 mh = { 
		.magic = MH_MAGIC_64,
		.filetype = MH_EXECUTE,
		.cputype = CPU_TYPE_X86_64,
		.cpusubtype = CPU_SUBTYPE_I386_ALL,
		.ncmds = 1,
		.sizeofcmds = sizeof(struct load_command) + sizeof(struct section_64),
		.flags = MH_NOUNDEFS,
	};

	struct load_command text_segment_load = {
		.cmd = LC_SEGMENT_64,
		//.cmdsize = sizeof(struct section_64)
		.cmdsize = sizeof(struct segment_command_64)
	};

	struct segment_command_64 segment64 = {
		.cmd = LC_SEGMENT_64,
		.cmdsize = sizeof(struct segment_command_64) + sizeof(struct section_64),
		.segname = "__TEXT",
		.vmaddr = 0,
		.vmsize = 4096,
		.fileoff = sizeof(struct mach_header_64) + sizeof(struct load_command) + sizeof(struct segment_command_64),
		.filesize = 4096,
		.maxprot = VM_PROT_EXECUTE | VM_PROT_READ,
		.initprot = VM_PROT_EXECUTE | VM_PROT_READ,
		.nsects = 1,
		.flags = SG_NORELOC,
	};

	struct section_64 code = {
		.sectname = "__text",
		.segname = "__TEXT",
		.addr = 0,
		.size = 4096,
		.offset = sizeof(struct mach_header_64) + sizeof(struct load_command),
		.align = 3,
		.reloff = 0,
		.nreloc = 0,
		.flags = S_REGULAR,
		.reserved1 = 0,
		.reserved2 = 0
	};

	char code_ops[] = {0xb8, 0x01, 0x00, 0x00, 0x02, 0xbf, 0x00, 0x00, 0x00, 0x00, 0x0f, 0x05};

	FILE *fp = fopen("output.macho","w");
	fwrite(&mh, sizeof(mh), 1, fp);
	fwrite(&text_segment_load, sizeof(text_segment_load), 1, fp);
	fwrite(&segment64, sizeof(segment64), 1, fp);
	fwrite(&code, sizeof(code), 1, fp);
	fwrite(code_ops, sizeof(mh) + sizeof(text_segment_load) + sizeof(segment64) + sizeof(code), 1, fp);
	fclose(fp);

}
