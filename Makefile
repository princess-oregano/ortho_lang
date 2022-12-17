BACK_END := back_end
FRONT_END := front_end
TREE := tree

TEST := test

SRCDIR = src
OBJDIR = obj

SRCDIR_BACK_END := $(addprefix $(SRCDIR)/, $(BACK_END))
SRCDIR_FRONT_END := $(addprefix $(SRCDIR)/, $(FRONT_END))
SRCDIR_TREE := $(addprefix $(SRCDIR)/, $(TREE))
OBJDIR_BACK_END := $(addprefix $(OBJDIR)/, $(BACK_END))
OBJDIR_FRONT_END := $(addprefix $(OBJDIR)/, $(FRONT_END))
OBJDIR_TREE := $(addprefix $(OBJDIR)/, $(TREE))

SRC := main.cpp file.cpp log.cpp
SRC_BACK_END := 
SRC_FRONT_END := lexer.cpp parser.cpp
SRC_TREE := tree.cpp tree_dump.cpp system.cpp

OBJ := $(addprefix $(OBJDIR)/, $(SRC:.cpp=.o))
OBJ_BACK_END := $(addprefix $(OBJDIR_BACK_END)/, $(SRC_BACK_END:.cpp=.o))
OBJ_FRONT_END := $(addprefix $(OBJDIR_FRONT_END)/, $(SRC_FRONT_END:.cpp=.o))
OBJ_TREE := $(addprefix $(OBJDIR_TREE)/, $(SRC_TREE:.cpp=.o))

CXX := g++
CXXFLAGS := -O3 -g -std=c++14 -fmax-errors=100 -Wall -Wextra                  \
	    -Weffc++ -Waggressive-loop-optimizations -Wc++0x-compat           \
	    -Wc++11-compat -Wc++14-compat -Wcast-align -Wcast-qual            \
	    -Wchar-subscripts -Wconditionally-supported -Wconversion          \
	    -Wctor-dtor-privacy -Wempty-body -Wfloat-equal                    \
	    -Wformat-nonliteral -Wformat-security -Wformat-signedness         \
	    -Wformat=2 -Winline -Wlarger-than=48000 -Wlogical-op              \
	    -Wmissing-declarations -Wnon-virtual-dtor -Wopenmp-simd           \
	    -Woverloaded-virtual -Wpacked -Wpointer-arith -Wredundant-decls   \
	    -Wshadow -Wsign-conversion -Wsign-promo -Wstack-usage=16000       \
	    -Wstrict-null-sentinel -Wstrict-overflow=2                        \
	    -Wsuggest-attribute=noreturn -Wsuggest-final-methods              \
	    -Wsuggest-final-types -Wsuggest-override -Wswitch-default         \
	    -Wswitch-enum -Wsync-nand -Wundef -Wunreachable-code -Wunused     \
	    -Wuseless-cast -Wvariadic-macros -Wno-literal-suffix              \
	    -Wno-missing-field-initializers -Wno-narrowing                    \
	    -Wno-old-style-cast -Wno-varargs -fcheck-new                      \
	    -fsized-deallocation -fstack-check -fstack-protector              \
	    -fstrict-overflow -flto-odr-type-merging                          \
	    -fno-omit-frame-pointer -fsanitize=alignment                      \
            -fsanitize=address -fsanitize=bool -fsanitize=bounds              \
	    -fsanitize=enum -fsanitize=float-cast-overflow                    \
	    -fsanitize=float-divide-by-zero                                   \
	    -fsanitize=integer-divide-by-zero -fsanitize=leak                 \
	    -fsanitize=nonnull-attribute -fsanitize=null                      \
	    -fsanitize=object-size -fsanitize=return                          \
	    -fsanitize=returns-nonnull-attribute -fsanitize=shift             \
	    -fsanitize=signed-integer-overflow                                \
	    -fsanitize=undefined -fsanitize=unreachable                       \
	    -fsanitize=vla-bound -fsanitize=vptr -fPIE -lm -pie
.SILENT:
all: out run

run:
	printf "%s\n" "Running..."
	./$(TEST)
	printf "%s\n" "Finished."

out: $(OBJDIR) $(OBJ) $(OBJ_BACK_END) $(OBJ_FRONT_END) $(OBJ_TREE)
	printf "%s\n" "Linking..."
	$(CXX) $(OBJ_BACK_END) $(OBJ_FRONT_END) $(OBJ_TREE) $(OBJ) -o $(TEST) $(CXXFLAGS)

$(OBJDIR)/%.o: $(SRCDIR)/%.cpp
	printf "%s\n" "Compiling $@..."
	$(CXX) -c $^ -o  $@ $(CXXFLAGS)

$(OBJDIR_BACK_END)/%.o: $(SRCDIR_BACK_END)/%.cpp
	printf "%s\n" "Compiling $@..."
	$(CXX) -c $^ -o  $@ $(CXXFLAGS)

$(OBJDIR_FRONT_END)/%.o: $(SRCDIR_FRONT_END)/%.cpp
	printf "%s\n" "Compiling $@..."
	$(CXX) -c $^ -o  $@ $(CXXFLAGS)

$(OBJDIR_TREE)/%.o: $(SRCDIR_TREE)/%.cpp
	printf "%s\n" "Compiling $@..."
	$(CXX) -c $^ -o  $@ $(CXXFLAGS)

$(OBJDIR):
	printf "%s\n" "Making $@/ directory..."
	mkdir $@
	mkdir $(OBJDIR_BACK_END)
	mkdir $(OBJDIR_FRONT_END)
	mkdir $(OBJDIR_TREE)

clean:
	printf "%s\n" "Removing $(OBJDIR)/ directory..."
	rm -rf $(OBJDIR)
	printf "%s\n" "Done."

distclean:
	printf "%s\n" "Removing built files..."
	rm -rf $(OBJDIR)
	printf "%s\n" "Done."

