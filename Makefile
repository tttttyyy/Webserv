# NAME = webserv
# CXX = c++
# CXXFLAGS =  -Iinclude -std=c++98 -Wall -Werror -Wextra -MMD -MP
# SRCS = $(wildcard src/*.cpp)
# # OBJS = $(SRCS:.cpp=.o)
# OBJS = $(patsubst src/%.cpp, ./$(DIR)/%.o, $(SRCS))

# HEADER += $(wildcard *.hpp)
# HEADER = $(wildcard include/*.hpp)

# DIR = objs
# DEPS = $(OBJS:%.o=%.d)
# RM = rm -rf

# # -include ${DEPS}

# all: $(DIR) $(NAME) 

# ./$(DIR)/%.o: src/%.cpp $(HEADER) Makefile# | $(DIR)
# 	$(CXX) ${CXXFLAGS} -o $@ -c $<

# $(NAME): $(OBJS)
# 	$(CXX) $(CXXFLAGS) $(OBJS) -o $(NAME)

# $(DIR):
# 	mkdir -p $(DIR)

# clean:
# 	$(RM) *.o *.d

# fclean: clean
# 	$(RM) $(NAME) $(DIR)

# re: fclean all

# .PHONY: all clean fclean re


NAME = webserver

UNAME := $(shell uname)

TMP = objs

# ifeq ($(UNAME), Linux)
# CXX = c++ -std=c++98
# else
CXX = c++ -std=c++98
# endif

CXXFLAGS = -I./includes -Wall -Wextra -Werror #-fsanitize=address -g

SRCS = $(wildcard src/*.cpp)

OBJS = $(patsubst src/%.cpp, ./$(TMP)/%.o, $(SRCS))

RM = rm -fr

HEADER = $(wildcard *.hpp)
HEADER += $(wildcard includes/*.hpp)

all: $(NAME)

./$(TMP)/%.o: ./src/%.cpp $(HEADER) Makefile
	$(CXX) $(CXXFLAGS) -o $@ -c $<

$(NAME): $(TMP) $(OBJS)
	$(CXX) $(CXXFLAGS) $(OBJS) -o $(NAME)

$(TMP):
	@mkdir $(TMP)

clean:
	$(RM) $(OBJS_DIR)
	$(RM) $(TMP)

fclean: clean
	$(RM) $(NAME)

re:	fclean all

.PHONY: all clean fclean re bonus
