#include <mach-o/loader.h>
#include <string.h>
#include <stdio.h>

int main() {
	struct mach_header_64 mh = { 
		.magic = MH_MAGIC_64,
		.filetype = MH_EXECUTE,
		.cputype = CPU_TYPE_X86_64,
		.cpusubtype = CPU_SUBTYPE_I386_ALL,
		.ncmds = 3,
		.sizeofcmds = (sizeof(struct segment_command_64) * 2)+ sizeof(struct section_64)
				+ sizeof(struct entry_point_command),
		.flags = MH_NOUNDEFS
	};

	char code_ops[] = { 0xb8, 0x01, 0x00, 0x00, 0x02, 0xbf, 0x00, 0x00, 0x00, 0x00, 0x0f, 0x05};

	struct segment_command_64 page_zero = {
		.cmd = LC_SEGMENT_64,
		.cmdsize = sizeof(struct segment_command_64),
		.segname = "__PAGEZERO",
		.vmaddr = 0,
		.vmsize = 4096,
		.fileoff = 0,
		.filesize = sizeof(code_ops) - 1,
		.maxprot = VM_PROT_EXECUTE | VM_PROT_READ,
		.initprot = VM_PROT_EXECUTE | VM_PROT_READ,
		.nsects = 0,
		.flags = SG_NORELOC
	};

	struct segment_command_64 segment64 = {
		.cmd = LC_SEGMENT_64,
		.cmdsize = sizeof(struct segment_command_64) + sizeof(struct section_64),
		.segname = "__TEXT",
		.vmaddr = 0x0000000100000000,
		.vmsize = 8192,
		.fileoff = 0,
		.filesize = sizeof(code_ops) - 1,
		.maxprot = VM_PROT_EXECUTE | VM_PROT_READ,
		.initprot = VM_PROT_EXECUTE | VM_PROT_READ,
		.nsects = 1,
		.flags = SG_NORELOC
	};


	struct section_64 code = {
		.sectname = "__text",
		.segname = "__TEXT",
		.addr = 0x0000000100000000 + 4096,
		.size = sizeof(code_ops) - 1, 
		.offset = sizeof(struct mach_header_64)
			  + sizeof(struct segment_command_64)
			  + sizeof(struct segment_command_64)
			  + sizeof(struct section_64)
			  + sizeof(struct entry_point_command),
		.align = 3,
		.reloff = 0,
		.nreloc = 0,
		.flags = S_REGULAR,
		.reserved1 = 0,
		.reserved2 = 0
	};

	struct entry_point_command entry = {
		.cmd = LC_MAIN,
		.cmdsize = sizeof(struct entry_point_command),
		.entryoff = 0,
		.stacksize = 0
	};


	FILE *fp = fopen("output.macho","w");
	fwrite(&mh, sizeof(mh), 1, fp);
	fwrite(&page_zero, sizeof(page_zero), 1, fp);
	fwrite(&segment64, sizeof(segment64), 1, fp);
	fwrite(&code, sizeof(code), 1, fp);
	fwrite(&entry, sizeof(entry), 1, fp);
	fwrite(code_ops, sizeof(code_ops), 1, fp);
	fclose(fp);

}
