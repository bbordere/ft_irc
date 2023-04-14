CXX = clang++

CXXFLAGS = -g3 -std=c++98 -Wall -Wextra -Werror -MMD -I includes
# CXXFLAGS = -g3  -std=c++98 -MMD -I includes

# FILES = srcs/protoServerSide.cpp
FILES = srcs/main.cpp srcs/Server.cpp srcs/User.cpp srcs/utils.cpp srcs/Channel.cpp

OBJS = $(FILES:.cpp=.o)

DEPS = $(FILES:.cpp=.d)

NAME = server

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

run: all
	@echo SERVER STARTED !
	./server

re: fclean all

.PHONY: all clean fclean re

-include $(FILES:%.cpp=%.d)