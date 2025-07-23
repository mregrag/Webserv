NAME	= webserv
CC	= c++
CFLAGS	= -Wall -Wextra -Werror -std=c++98

SRC	= 	src/main.cpp \
		src/Client.cpp \
		src/CGIHandler.cpp \
		src/ConfigParser.cpp \
		src/Epoll.cpp \
		src/HTTPRequest.cpp \
		src/HTTPResponse.cpp \
		src/Logger.cpp \
		src/LocationConfig.cpp \
		src/ServerConfig.cpp \
		src/ServerManager.cpp \
		src/Utils.cpp

HPP		= include/Client.hpp \
		include/CGIHandler.hpp \
		include/ConfigParser.hpp \
		include/Epoll.hpp \
		include/HTTPRequest.hpp \
		include/HTTPResponse.hpp \
		include/Logger.hpp \
		include/LocationConfig.hpp \
		include/ServerConfig.hpp \
		include/ServerManager.hpp \
		include/Utils.hpp

OBJ		= $(SRC:.cpp=.o)

all: $(NAME)

$(NAME): $(OBJ)
	$(CC) $(CFLAGS) -o $(NAME) $(OBJ)

%.o: %.cpp $(HPP)
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	rm -f $(OBJ)

fclean: clean
	rm -f $(NAME)

re: fclean all
