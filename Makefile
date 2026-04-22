

$(NAME), all, clean, fclean and re

NAME = Codexion

CC = gcc -pthread mon_programme.c -o mon_programme

FLAGS = --Wall --Wextra --Werror


.PHONY : all, clean, fclean, re