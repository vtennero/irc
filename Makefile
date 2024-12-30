NAME			= 	ircserv

SOURCE			= 	main.cpp Server.cpp Client.cpp NickCommandHandler.cpp QuitCommandHandler.cpp UserCommandHandler.cpp Message.cpp Channel.cpp PrivmsgCommandHandler.cpp WhoisCommandHandler.cpp ModeCommandHandler.cpp PingCommandHandler.cpp PongCommandHandler.cpp ServerMessageHandler.cpp
# JoinCommandHandler.cpp PartCommandHandler.cpp ListCommandHandler.cpp NamesCommandHandler.cpp KickCommandHandler.cpp TopicCommandHandler.cpp InviteCommandHandler.cpp

OBJECT			=	$(SOURCE:.cpp=.o)

COMPILE			= 	c++

FLAGS			= 	-Wall -Wextra -Werror -std=c++98

RESET			= 	\033[0m
GREEN 			= 	\033[38;5;46m
WHITE 			= 	\033[38;5;15m
GREY 			= 	\033[38;5;8m
ORANGE 			= 	\033[38;5;202m
RED 			= 	\033[38;5;160m

all: $(NAME)

%.o: %.cpp
	@echo "$(GREY)Compiling...				$(WHITE)$<"
	@$(COMPILE) $(FLAGS) -c $< -o $@

$(NAME): $(OBJECT)
	@echo "$(GREEN)----------------------------------------------------"
	@$(COMPILE) $(FLAGS) $(OBJECT) -o $(NAME)
	@echo "Executable				./$(NAME) $(RESET)"

clean:
	@echo "$(RED)----------------------------------------------------"
	@/bin/rm -f $(OBJECT)
	@echo "$(WHITE)REMOVED O-FILES $(RESET)"

fclean: clean
	@echo "$(RED)----------------------------------------------------"
	@/bin/rm -f $(NAME)
	@echo "$(WHITE)REMOVED EXECUTABLE FILE $(RESET)"

re: fclean all

.PHONY: all clean fclean re
