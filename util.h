void dump_hex(char *str, long long start, long long size);

int read_mapping (FILE *mapfile, 
	      unsigned long long *addr, 
	      unsigned long long *endaddr, 
	      char *permissions, 
	      unsigned long long *offset, 
	      char *device, 
	      unsigned long long *inode, 
	      char *filename);

unsigned long long str2num(char *str);
