CXX = clang++

CXXFLAGS = -g3 -std=c++98 -Wall -Wextra -Werror -MMD -I includes

FILES = bot.cpp

OBJS = $(FILES:.cpp=.o)

DEPS = $(FILES:.cpp=.d)

NAME = beBot

%.o: %.cpp
	$(CXX) $(CXXFLAGS) -c $< -o $@

all: $(NAME)

std: $(OBJS)
	$(CXX) $(CXXFLAGS) $(OBJS) -o $(NAME)

$(NAME): $(OBJS)
	$(CXX) $(CXXFLAGS) $(OBJS) -o $(NAME)

clean:
	rm -f $(OBJS)
	rm -f $(DEPS)

fclean: clean
	rm -f $(NAME)

re: fclean all

.PHONY: all clean fclean re

-include $(FILES:%.cpp=%.d)