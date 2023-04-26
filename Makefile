CXX = c++

CXXFLAGS = -g3 -std=c++98 -Wall -Wextra -Werror -MMD -I includes

SRCS_DIR = srcs/
OBJS_DIR = objs/
DEPS_DIR = deps/

FILES = srcs/main.cpp srcs/Server.cpp srcs/User.cpp srcs/utils.cpp srcs/Channel.cpp srcs/command.cpp
OBJS = $(patsubst $(SRCS_DIR)%.cpp, $(OBJS_DIR)%.o, $(FILES))
DEPS = $(patsubst $(SRCS_DIR)%.cpp, $(DEPS_DIR)%.d, $(FILES))

NAME = ircserv

$(OBJS_DIR)%.o: $(SRCS_DIR)%.cpp
	@mkdir -p $(OBJS_DIR)
	@mkdir -p $(DEPS_DIR)
	$(CXX) $(CXXFLAGS) -c -MF $(patsubst $(OBJS_DIR)%.o, $(DEPS_DIR)%.d, $@) $< -o $@

all: $(NAME)

$(NAME): $(OBJS)
	$(CXX) $(CXXFLAGS) $(OBJS) -o $(NAME)

clean:
	rm -Rf $(OBJS_DIR) $(DEPS_DIR)

fclean: clean
	rm -f $(NAME)

re: fclean all

.PHONY: all clean fclean re

-include $(DEPS)