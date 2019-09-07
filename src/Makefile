CC   = clang
NAME = cbm

#Additional Libraries
LUA_INC= ./lua/install/include
LUA_LIB= ./lua/install/lib

BCRYPT_INC= ./bcrypt
BCRYPT_LIB= ./bcrypt

JANSSON_INC= ./jansson
JANSSON_LIB= ./jansson/lib

INCLUDES=  -I$(LUA_INC) -I$(BCRYPT_INC) -I$(JANSSON_INC)
LIBRARIES= -L$(LUA_LIB) -L$(BCRYPT_LIB) -L$(JANSSON_LIB)

W_FLAGS = -Wall -Werror -Wshadow -Wformat-security -Wimplicit-function-declaration -Wpointer-arith -Wcast-align -Wredundant-decls  -pedantic

C_FLAGS = -g3 $(W_FLAGS) $(INCLUDES) -std=c11
L_FLAGS = -lz -llua -lbcrypt -ljansson -lpthread -lm $(LIBRARIES)
D_FLAGS = -ggdb $(W_FLAGS)

O_FILES = socket.o io.o strings.o utils.o interpret.o help.o  \
	  action_safe.o mccp.o save.o event.o event-handler.o \
	  list.o stack.o handler_connections.o accounts.o \
	  handler_json.o comm.o colors.o world.o objects.o \
	  tables.o action_objects.c action_combat.c handler_lua.o \
	  reset.o action_wiz.o combat.o build.o

all: $(O_FILES)
	rm -f $(NAME)
	$(CC) -o $(NAME) $(O_FILES) $(L_FLAGS) $(C_FLAGS) $(D_FLAGS)

.c.o: all
	@$(CC) -c $(C_FLAGS) $<

clean:
	@echo Cleaning code $< ...
	@rm -rf *.o
	@rm -f $(NAME)
	@rm -f *.*~
