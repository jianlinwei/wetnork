# usage: $(call make-depend,source-file,object-file,depend-file)
define make-depend
	@$(CPP) -MM -MP -MT $2 $(CFLAGS) $1 | sed -e 's@^\(.*\)\.o:@\1.d \1.o:@' > $3
endef

define dirname
	$(patsubst %/,%,$(dir $1))
endef
