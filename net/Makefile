DIRS += $(OBJDIR)/net

net_SRC = $(wildcard net/*.cpp)

net_OBJ = $(patsubst %.cpp,$(OBJDIR)/%.o,$(net_SRC))

DEP_SRC += $(net_SRC)

$(OBJDIR)/libnet.a: $(net_OBJ)
	$(AR) -rcs $@ $^
