DIRS += $(OBJDIR)/common

common_SRC = $(wildcard common/*.cpp)

common_OBJ = $(patsubst %.cpp,$(OBJDIR)/%.o,$(common_SRC))

DEP_SRC += $(common_SRC)

$(OBJDIR)/libcommon.a: $(common_OBJ)
	$(AR) -rcs $@ $^
