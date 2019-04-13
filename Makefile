ALL_OBJ   =  logger.o fsmon.o find_fmt_func.o
# Do not use builtin function so that we can inject it without name conflicts
CFLAGS  = -fPIC -g -fno-builtin
all: $(ALL_OBJ)
	make -C testcases
	gcc $(CFLAGS) -shared -ldl -o fsmon.so $(ALL_OBJ)

clean:
	rm $(ALL_OBJ)
	make -C testcases clean
	rm fsmon.so

va_macro.h: gen_va_macro.sh
	./gen_va_macro.sh > va_macro.h

logger.o: common.h logger.c
	gcc $(CFLAGS) -c logger.c
fsmon.o: fsmon.c common.h va_macro.h
	gcc $(CFLAGS) -c fsmon.c
find_fmt_func.o: common.h find_fmt_func.c
	gcc $(CFLAGS) -c find_fmt_func.c