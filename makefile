
SRCS = client.c create.c mytest.c server.c
CC = cc
CFLAGS = -Wall -Wextra -Werror -DTIMES
RM = rm -f
OBJS = ${SRCS:.c=.o}
INCS = project.h
LIBC = ar rc
NAME = libmytest.a

all : $(NAME)
%.o : %.c
    $(CC) $(CFLAGS) -c $< -o $@ -I $(INCS)
$(NAME) : $(OBJS)
    $(LIBC) $(NAME) $(OBJS)
clean:
    $(RM) $(OBJS)
fclean: clean
    $(RM) $(NAME)
re: fclean all
bonus:
    make BONUS=1 all
.PHONY : clean fclean re all