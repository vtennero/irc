#!/bin/bash

#https://www.linuxquestions.org/questions/programming-9/dynamically-supply-input-to-netcat-in-a-script-793526/

# Function to generate a random 4-letter alphabetic name
# Determine the operating system
if [[ $(uname) == "Darwin" ]]; then
  generate_random_name() {
    cat /dev/urandom | LC_CTYPE=C tr -dc 'a-zA-Z' | fold -w 4 | head -n 1
  }
else
  generate_random_name() {
    cat /dev/urandom | tr -dc 'a-zA-Z' | fold -w 4 | head -n 1
  }
fi

# SERVER='10.12.4.4' #'localhost'
SERVER='localhost' #'localhost'
PORT='6667'
INTERVAL='1'
NAME=$(generate_random_name)
REAL_NAME=$NAME
CHANNEL='#wez'
PASSWORD='password123'


input() {
	#Connect to the server, Identify, and join channel
  echo -e "CAP LS\r";
  echo -e "PASS $PASSWORD\r";
	echo -e "NICK $NAME\r";
	echo -e "USER $NAME 0 * :$REAL_NAME\r";
  echo -e "CAP END\r";
	echo -e "JOIN $CHANNEL\r";

  # Read user input and pass it to the server
  while read -r irc_command; do
    echo -e "$irc_command\r"
  done
  }
#Prevents a disconnect from a ping timeout
#	while true; do
#		pong
#	done;
#	}

#-i $INTERVAL

function my_irc {
	input | nc  $SERVER $PORT
}

my_irc
