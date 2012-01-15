DIRS += $(OBJDIR)/host

host_SRC = $(wildcard host/*.cpp)

host_OBJ = $(patsubst %.cpp,$(OBJDIR)/%.o,$(host_SRC))

DEP_SRC += $(host_SRC)

$(OBJDIR)/libhost.a: $(host_OBJ)
	@echo -e "[AR]\t" $@
	$(SILENT)$(AR) -rcs $@ $^
