BACK_END := elf_back_end
FRONT_END := front_end
TREE := tree
STACK := stack

TEST_AST := ast
TEST_GEN := gen

SRCDIR = src
OBJDIR = obj

SRCDIR_BACK_END := $(addprefix $(SRCDIR)/, $(BACK_END))
SRCDIR_FRONT_END := $(addprefix $(SRCDIR)/, $(FRONT_END))
SRCDIR_TREE := $(addprefix $(SRCDIR)/, $(TREE))
SRCDIR_STACK := $(addprefix $(SRCDIR)/, $(STACK))

OBJDIR_BACK_END := $(addprefix $(OBJDIR)/, $(BACK_END))
OBJDIR_FRONT_END := $(addprefix $(OBJDIR)/, $(FRONT_END))
OBJDIR_TREE := $(addprefix $(OBJDIR)/, $(TREE))
OBJDIR_STACK := $(addprefix $(OBJDIR)/, $(STACK))

SRC := args.cpp file.cpp log.cpp identifiers.cpp 
SRC_BACK_END := main.cpp encode.cpp generator.cpp symbol.cpp
SRC_FRONT_END := main.cpp lexer.cpp parser.cpp
SRC_TREE := tree.cpp tree_dump.cpp system.cpp
SRC_STACK := stack.cpp error.cpp 

OBJ := $(addprefix $(OBJDIR)/, $(SRC:.cpp=.o))
OBJ_BACK_END := $(addprefix $(OBJDIR_BACK_END)/, $(SRC_BACK_END:.cpp=.o))
OBJ_FRONT_END := $(addprefix $(OBJDIR_FRONT_END)/, $(SRC_FRONT_END:.cpp=.o))
OBJ_TREE := $(addprefix $(OBJDIR_TREE)/, $(SRC_TREE:.cpp=.o))
OBJ_STACK := $(addprefix $(OBJDIR_STACK)/, $(SRC_STACK:.cpp=.o))

CXX := g++
CXXFLAGS := -O3 -g -std=c++14 -fmax-errors=100 -Wall -Wextra                  \
	    -Weffc++ -Waggressive-loop-optimizations -Wc++0x-compat           \
	    -Wc++11-compat -Wc++14-compat -Wcast-align -Wcast-qual            \
	    -Wchar-subscripts -Wconditionally-supported -Wconversion          \
	    -Wctor-dtor-privacy -Wempty-body -Wfloat-equal                    \
	    -Wformat-nonliteral -Wformat-security -Wformat-signedness         \
	    -Wformat=2 -Winline -Wlarger-than=72000 -Wlogical-op              \
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
	#printf "%s\n" "Running..."
	#printf "%s\n" "Building AST..."
	#./$(TEST_AST) test.txt
	#printf "%s\n" "Generating ASM..."
	#./$(TEST_GEN) test.ast
	#printf "%s\n" "Running ASM..."
	#./asm test.asm
	#printf "%s\n" "Running CPU..."
	#./cpu test.ast
	#printf "%s\n" "Finished."

out: $(OBJDIR) $(OBJ) $(OBJ_BACK_END) $(OBJ_FRONT_END) $(OBJ_TREE) $(OBJ_STACK)
	printf "%s\n" "Linking..."
	$(CXX) $(OBJ_FRONT_END) $(OBJ_TREE) $(OBJ) -o $(TEST_AST) $(CXXFLAGS)
	$(CXX) $(OBJ_BACK_END) $(OBJ_TREE) $(OBJ_STACK) $(OBJ) -o $(TEST_GEN) $(CXXFLAGS)

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

$(OBJDIR_STACK)/%.o: $(SRCDIR_STACK)/%.cpp
	printf "%s\n" "Compiling $@..."
	$(CXX) -c $^ -o  $@ $(CXXFLAGS)

$(OBJDIR):
	printf "%s\n" "Making $@/ directory..."
	mkdir $@
	mkdir $(OBJDIR_BACK_END)
	mkdir $(OBJDIR_FRONT_END)
	mkdir $(OBJDIR_TREE)
	mkdir $(OBJDIR_STACK)

clean:
	printf "%s\n" "Removing $(OBJDIR)/ directory..."
	rm -rf $(OBJDIR)
	printf "%s\n" "Done."

distclean:
	printf "%s\n" "Removing built files..."
	rm -rf $(OBJDIR)
	rm -rf dmp
	rm test.ast test.asm test.mur
	rm listing.txt
	rm log.html
	rm $(TEST_AST)
	rm $(TEST_GEN)
	printf "%s\n" "Done."

