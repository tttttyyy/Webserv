NAME = webserver
CXX = c++
CXXFLAGS =  -Iinclude -std=c++98 -Wall -Werror -Wextra -MMD -MP
SRCS = $(wildcard src/*.cpp)
# OBJS = $(SRCS:.cpp=.o)
OBJS = $(patsubst src/%.cpp, ./$(DIR)/%.o, $(SRCS))

HEADER += $(wildcard *.hpp)
HEADER = $(wildcard include/*.hpp)

DIR = objs
DEPS = $(OBJS:%.o=%.d)
RM = rm -rf

# -include ${DEPS}

all: $(DIR) $(NAME) 

./$(DIR)/%.o: src/%.cpp $(HEADER) Makefile# | $(DIR)
	$(CXX) ${CXXFLAGS} -o $@ -c $<

$(NAME): $(OBJS)
	$(CXX) $(CXXFLAGS) $(OBJS) -o $(NAME)

$(DIR):
	mkdir -p $(DIR)

clean:
	$(RM) *.o *.d

fclean: clean
	$(RM) $(NAME) $(DIR)

re: fclean all

.PHONY: all clean fclean re
