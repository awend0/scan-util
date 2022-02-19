NAME    =       scan_util.exe
SRCS    =       src/main.cpp \
				src/Scanner.cpp
INCDIR	=		inc
FLAGS   =       -std=c++17 -Wall -Wextra -Werror -O3 -I$(INCDIR)
OBJS    =       $(SRCS:cpp=o)

all: $(NAME)

$(NAME): $(OBJS)
		g++ $(FLAGS) -o $(NAME) $(SRCS)

%.o: %.cpp
		g++ $(FLAGS) -c $< -o $@

clean:
		rm -rf $(OBJS)

fclean: clean
		rm -rf $(NAME)

re: fclean all