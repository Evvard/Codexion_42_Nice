
NAME    = codexion

CC      = gcc
CFLAGS  = -Wall -Wextra -Werror -pthread

SRC     = parser.c main.c thread.c codexion_utils.c codexion_actions.c
OBJ     = $(SRC:.c=.o)

all: $(NAME)

$(NAME): $(OBJ)
	$(CC) $(CFLAGS) $(OBJ) -o $(NAME)

%.o: %.c codexion.h
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	rm -f $(OBJ)

fclean: clean
	rm -f $(NAME)

re: fclean all

.PHONY: all clean fclean re