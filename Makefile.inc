subdirs = $(sort $(patsubst %,_isix_subdir_%, $(SUBDIR-y)))

objs = $(sort $(patsubst %,_isix_obj_%, $(OBJ-y)))
objs_all: objs_ok subdirs_ok

subdirs_ok: $(subdirs)
	$(LD) -r -o $(OBJ_TARGET) $(OBJ_LINK)

objs_ok: $(objs)

$(objs) :
	$(MAKE) $(patsubst _isix_obj_%,%,$@)

$(subdirs) : 
	$(MAKE) -C $(patsubst _isix_subdir_%,%,$@)



%.dep: %.c
	$(CC) -M $(CFLAGS) $< > $@.$$$$; \
	sed 's,\($*\)\.o[ :]*,\1.o $@ : ,g' < $@.$$$$ > $@; \
	rm -f $@.$$$$

%.dep: %.S
	$(CC) -M $(ASFLAGS) $< > $@.$$$$; \
	sed 's,\($*\)\.o[ :]*,\1.o $@ : ,g' < $@.$$$$ > $@; \
	rm -f $@.$$$$

%.o : %.S
	@echo "Assembling..."
	$(CC) -c $(ASFLAGS)  $< -o $@ 


%.o : %.c	
	@echo "Compiling C..."
	$(CC) -c $(CFLAGS) $< -o $@

-include $(OBJ-y:.o=.dep)
