# **************************************************************************** #
#                                                                              #
#                                                         :::      ::::::::    #
#    Makefile                                           :+:      :+:    :+:    #
#                                                     +:+ +:+         +:+      #
#    By: kbolon <kbolon@42.fr>                      +#+  +:+       +#+         #
#                                                 +#+#+#+#+#+   +#+            #
#    Created: 2025/04/14 17:38:09 by kbolon            #+#    #+#              #
#    Updated: 2025/05/06 16:22:23 by kbolon           ###   ########.fr        #
#                                                                              #
# **************************************************************************** #

NAME = webserv

SRC_DIR = srcs
INC_DIR = includes
OBJ_DIR = obj

SRCS = 	server_main.cpp \
		$(SRC_DIR)/ServerSocket.cpp $(SRC_DIR)/ClientConnection.cpp $(SRC_DIR)/helperFunction.cpp \
		$(SRC_DIR)/ConfigParser.cpp
		

#patsubst is short for pattern substitution, works with items in multiple folders
OBJS = $(SRCS:.cpp=.o)
OBJS := $(patsubst %, $(OBJ_DIR)/%,$(OBJS))

CXX = c++
CXXFLAGS = -Wall -Wextra -Werror -std=c++98 -I $(INC_DIR)

all: $(NAME) client

$(NAME): $(OBJS)
	$(CXX) $(CXXFLAGS) -o $@ $^

client: test_client.cpp $(SRC_DIR)/ConfigParser.cpp $(SRC_DIR)/helperFunction.cpp
	$(CXX) $(CXXFLAGS) -o $@ $^

$(OBJ_DIR)/%.o: %.cpp 
	@mkdir -p $(dir $@)
	$(CXX) $(CXXFLAGS) -c $< -o $@ 

clean:
	rm -rf $(OBJ_DIR)

fclean: clean
	rm -f $(NAME) client

re: fclean all

.PHONY: all clean fclean re