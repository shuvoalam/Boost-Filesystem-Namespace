#ifndef GLOBAL_DEFINES_HPP_INCLUDED
#define GLOBAL_DEFINES_HPP_INCLUDED

#define LETTERS "qwertyuiopasdfghjklzxcvbnmQWERTYUIOPASDFGHJKLZXCVBNM"
#define NUMBERS "1234567890"
#define SPECIALS "`~!@#$%^&*()-_=+[{]}\\|;:\'\",<.>/? "

/* non-control keys (keys != 27):*/
#define BACKSPACE_KEY 127
#define ENTER_KEY 10

/* Control characters (int(char) == 27), as represented by a vector of multiple return codes: */
#define LEFT_KEY std::vector<int>({27, 91, 68})
#define RIGHT KEY std::vector<int>({27, 91, 67})
#define UP_KEY std::vector<int>({27, 91, 65})
#define DOWN_KEY std::vector<int>({27, 91, 66})
#define HOME_KEY std::vector<int>({27, 79, 72})
#define END_KEY std::vector<int>({27, 79, 70})
#define PGUP_KEY std::vector<int>({27, 91, 53, 126})
#define PGDOWN_KEY std::vector<int>({27, 91, 54, 126})
#define DELETE_KEY std::vector<int>({27, 91, 51, 126})

#endif
