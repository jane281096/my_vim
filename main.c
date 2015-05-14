#include <math.h>
#include <curses.h>
#include <termios.h>
#include <sys/ioctl.h>
#include <signal.h>
#include <stdlib.h>
#include <fcntl.h>
#include <string.h>
#include <unistd.h>

struct list
{
    char val;
    struct list* next;
};

typedef struct list list;

list* plus_next(list* beg,char c)
{
    if (beg != NULL)
    {
        list* copy = beg->next;
        beg->next = (list*) malloc(sizeof(list));
        beg->next->val = c;
        beg->next->next = copy;
        return beg->next;
    }
    else
    {
        beg = (list*) malloc(sizeof(list));
        if (beg == NULL)
        {
            perror("malloc");
        }
        beg->val = c;
        beg->next = NULL;
        return beg;
    }
}

void delete(list* beg)
{
    list* copy;
    if (beg != NULL)
    {
        while(1)
        {
            copy = beg->next;
            free(beg);
            if (copy == NULL)
            {
                break;
            }
            beg = copy;
        }
    }
}

struct list_p
{   
        list* val;
        struct list_p* next;
};

typedef struct list_p list_p;

list_p* plus_next_p(list_p* beg, list* c)
{   
    //printf("in plus_next_p\n");
    if (beg != NULL)
    {   
        list_p* copy = beg->next;
        beg->next = (list_p*) malloc(sizeof(list_p));
        beg->next->val = c;
        beg->next->next = copy;
        return beg->next;
    }
    else
    {   
        //printf("beg is NULL\n");
        beg = (list_p*) malloc(sizeof(list_p));
        if (beg == NULL)
        {   
            perror("malloc");
        }
        beg->val = c;
        //printf("val added\n");
        beg->next = NULL;
        return beg;
    }
}

void delete_p(list_p* beg)
{
    list_p* copy;
    if (beg != NULL)
    {   
        while(1)
        {   
            copy = beg->next;
            delete(beg->val);
            free(beg);
            if (copy == NULL)
            {
                break;
            }
            beg = copy;
        }   
    }   
}

int mode = 0;
int numbers_key = 0;
int save_key = 1;
list_p* list_file;
int** mfile;
int* view;
int win_beg_y;
//int tab_col = 5;

void print_file(int str_beg, list_p* list_file, int* view)
{
    list_p* list_file_copy = list_file;
    list* string_copy;
    clear();
    move(0, 0);
    int counter_y_win = 0;
    int counter_x_win = 0;
    int counter_str_file = 0;
    struct winsize size;
    ioctl(fileno(stdout), TIOCGWINSZ, (char*) &size);
    for (int i = 0; i < size.ws_row; i++)
    {
        view[i] = -1;
    }
    int win_y, win_x;
    while ((list_file_copy != NULL) && (counter_y_win < size.ws_row - 1))
    {
        if (counter_str_file >= str_beg)
        {
            if (numbers_key != 0)
            {
                printw("%d", counter_str_file);
                int cnt_copy = counter_str_file;
                if (cnt_copy == 0)
                {
                    cnt_copy = 1;
                }
                while (cnt_copy / ((int) pow(10, numbers_key - 1)) == 0)
                {
                    printw(" ");
                    cnt_copy *= 10;
                }
            }
            string_copy = list_file_copy->val;
            if (numbers_key != 0)
            {
                counter_x_win = numbers_key - 1;////
            }
            else
            {
                counter_x_win = 0;
            }
            while ((string_copy != NULL) && (counter_y_win < size.ws_row - 1))
            {
                if (counter_x_win + 1 == size.ws_col)
                {
                    view[counter_y_win] = counter_str_file;
                    ++counter_y_win;
                    if (numbers_key != 0)
                    {
                        for (int i = 0; i < numbers_key; i++)
                        {
                            printw(" ");
                        }
                    }
                    counter_x_win = 0;
                }
                else
                {
                    ++counter_x_win;
                }
                if (counter_y_win < size.ws_row - 1)
                {
                    printw("%c", string_copy->val);
                }
                string_copy = string_copy->next;
            }
            view[counter_y_win] = counter_str_file;
            ++counter_y_win;
            getyx(stdscr, win_y, win_x);
            if (win_y < counter_y_win)
            {
                printw("\n");
            }
        }
        ++counter_str_file;
        list_file_copy = list_file_copy->next;
    }
    move(size.ws_row - 1, 0);
    start_color();
    init_pair(1, COLOR_BLACK, COLOR_CYAN);
    attron(COLOR_PAIR(1));
    if (mode == 1)
    {
        printw("***** edit mode *****");
    }
    if (mode == 0)
    {
        printw("***** read mode *****");
    }
    for (int i = 21; i < size.ws_col; i++)
    {
        printw(" ");
    }
    attroff(COLOR_PAIR(1));
}

void sig_winch(int signo)
{
    int win_y, win_x;
    getyx(stdscr, win_y, win_x);
    struct winsize size;
    ioctl(fileno(stdout), TIOCGWINSZ, (char *) &size);
    resizeterm(size.ws_row, size.ws_col);
    print_file(win_beg_y, list_file, view);
    move(win_y, win_x);
}

list_p* init_file(int fd, int** mfile)
{
    char stream;
    int er_read = read(fd, &stream, sizeof(char));
    int first_key = 1;
    
    list_p* res = NULL;
    list_p* res_copy = NULL;
    list* res_val_copy;
    mfile[0][0] = 0;
    while (er_read != 0)
    {
        if ((stream == '\n') || (first_key == 1))
        {
            if (first_key == 1)
            {
                res = plus_next_p(res, NULL);
                res_copy = res;
                first_key = 0;
            }
            else
            {
                res_copy = plus_next_p(res_copy, NULL);
            }
            mfile[0][0] = mfile[0][0] + 1;
        }
        if ((stream >= 32) && (stream <= 127))
        {
            if (res_copy->val == NULL)
            {
                res_copy->val = plus_next(res_copy->val, stream);
                res_val_copy = res_copy->val;
            }
            else
            {
                res_val_copy = plus_next(res_val_copy, stream);
            }
        }
        er_read = read(fd, &stream, sizeof(char));
    }
    mfile[1] = (int*) malloc(sizeof(int) * mfile[0][0]);
    res_copy = res;
    list* string_copy;
    int counter_string;
    int counter_file = 0;
    while (res_copy != NULL)
    {
        string_copy = res_copy->val;
        counter_string = 0;
        while(string_copy != NULL)
        {
            string_copy = string_copy->next;
            ++counter_string;
        }
        mfile[1][counter_file] = counter_string;
        ++counter_file;
        res_copy = res_copy->next;
    }
    return res;
}

void print_cat(struct winsize size)
{
    clear();
    move(size.ws_row / 2 - 10, size.ws_col / 2 - 40);
    printw("                _\n");
    move(size.ws_row / 2 - 9, size.ws_col / 2-40);
    printw("                \\`*-.\n");
    move(size.ws_row / 2  - 8, size.ws_col / 2- 40);
    printw("                 )  _`-. \n");
    move(size.ws_row / 2 - 7, size.ws_col / 2- 40);
    printw("                .  : `. . \n");
    move(size.ws_row / 2 - 6, size.ws_col / 2- 40);
    printw("                : _   '  \\ \n");
    move(size.ws_row / 2 - 5, size.ws_col / 2- 40);
    printw("                ; *` _.   `*-._\n");
    move(size.ws_row / 2 - 4, size.ws_col / 2- 40);
    printw("                `-.-'          `-.\n");
    move(size.ws_row / 2 - 3, size.ws_col / 2- 40);
    printw("                  ;       `       `.\n");
    move(size.ws_row / 2 - 2, size.ws_col / 2- 40);
    printw("                  :.       .        \\\n");
    move(size.ws_row / 2 - 1, size.ws_col / 2- 40);
    printw("                  . \\  .   :   .-'   .\n");
    move(size.ws_row / 2 , size.ws_col / 2- 40);
    printw("                  '  `+.;  ;  '      :\n");
    move(size.ws_row / 2 + 1, size.ws_col / 2- 40);
    printw("                  :  '  |    ;       ;-.\n");
    move(size.ws_row / 2 + 2, size.ws_col / 2- 40);
    printw("                  ; '   : :`-:     _.`* ;\n");
    move(size.ws_row / 2 + 3, size.ws_col / 2- 40);
    printw("         [bug] .*' /  .*' ; .*`- +'  `*' \n");
    move(size.ws_row / 2 + 4, size.ws_col / 2- 40);
    printw("               `*-*   `*-*  `*-*'\n");
    move(size.ws_row / 2 - 14, size.ws_col / 2 - 29);
    printw("Welcome to \"Cute VIM\"!\n");
    move(size.ws_row/ 2 - 12, size.ws_col / 2 - 23);
    printw("MEOW");
    move(size.ws_row / 2 + 6, size.ws_col / 2 - 25);
    printw("PRESS ANY KEY");
    refresh();
    getch();
}

int main(int argc, char** argv)
{
    int er = open("/Users/evgeniatveritinova1/my_vim/er", O_RDWR);
    if (er == -1)
    {
        perror("open");
    }
    if (dup2(er, fileno(stderr)) < 0)
    {
        perror("dup2");
    }
    ftruncate(er, 0);

    int tab_col = 3;

    FILE* vimrc = fopen("/Users/evgeniatveritinova1/my_vim/vimrc", "r");
    fscanf(vimrc, "%d", &tab_col);
    
    initscr();
    int fd = open(argv[1], O_RDWR | O_CREAT,0666);
    if (fd == -1)
    {
        perror("open");
    }
    signal(SIGWINCH, sig_winch);
    int key;
    keypad(stdscr, TRUE);
    int win_x, win_y;
    struct winsize size;
    ioctl(fileno(stdout), TIOCGWINSZ, (char*) &size);
    //list_p* list_file;
    //int** mfile;
    mfile = (int**) malloc(sizeof(int*) * 2);
    mfile[0] = (int*) malloc(sizeof(int));
    mfile[1] = NULL;
    list_file = init_file(fd, mfile);
    view = (int*) malloc(sizeof(int) * size.ws_row);
    
    print_cat(size);
    
    start_color();
    init_pair(2, COLOR_RED, COLOR_BLACK); 
    init_pair(3, COLOR_WHITE, COLOR_BLACK);

    print_file(0, list_file, view);
    win_beg_y = 0;
    char command[5];
    move(0, numbers_key);

    while (1)
    {
        if (numbers_key != 0)
        {
            numbers_key = (int) log10(mfile[0][0]) + 1;
        }
        noecho();
        ioctl(fileno(stdout), TIOCGWINSZ, (char*) &size); 
        key = getch();
        if (key == KEY_DOWN) //      ,    
        {
            getyx(stdscr, win_y, win_x);
            if (view[win_y] + 1 < mfile[0][0]) //       
            {
                if (win_y == size.ws_row - 2) // e     
                {
                    print_file(win_beg_y + 1, list_file, view);
                    ++win_beg_y;
                    if (mfile[1][view[size.ws_row - 2]] == 0)
                    {
                        move(size.ws_row - 2, numbers_key);
                    }
                    else
                    {
                        if (mfile[1][view[size.ws_row - 2]] > size.ws_col) //       
                        {
                            if (mfile[1][view[size.ws_row - 2]] - size.ws_col > win_x)
                            {
                                move(size.ws_row - 2, win_x);
                            }
                            else
                            {
                                move(size.ws_row - 2, mfile[1][view[size.ws_row - 2]] - size.ws_col - 1 + numbers_key);
                            }
                        }
                        else
                        {
                            if (mfile[1][view[size.ws_row - 2]] > win_x)
                            {
                                move(size.ws_row - 2, win_x);
                            }
                            else
                            {
                                move(size.ws_row - 2, mfile[1][view[size.ws_row - 2]] - 1 + numbers_key);
                            }
                        }
                    }
                }
                else //       
                {
                    //      ,     
                    int beg = view[win_y];
                    int win_y_copy = win_y;
                    while (beg == view[win_y_copy])
                    {
                        ++win_y_copy;
                    }
                    if(mfile[1][view[win_y_copy]] == 0) //     
                    {
                        move(win_y_copy, numbers_key);
                    }
                    else
                    {
                        if (mfile[1][view[win_y_copy]] - 1 >= win_x) //      ,    
                        {
                            move(win_y_copy, win_x);
                        }
                        else
                        {
                            move(win_y_copy, mfile[1][view[win_y_copy]] - 1 + numbers_key);
                        }
                    }
                }
            }
        }
        if (key == KEY_UP) //      ,    6
        {
            getyx(stdscr, win_y, win_x);
            if (view[win_y] - 1 >= 0) //       
            {
                if (win_y == 0) //      
                {
                    print_file(win_beg_y - 1, list_file, view);
                    --win_beg_y;
                    if (mfile[1][view[0]] == 0)
                    {
                        move(0, numbers_key);
                    }
                    else
                    {
                        if (mfile[1][view[0]] > win_x)
                        {
                            move(0, win_x);
                        }
                        else
                        {
                            move(0, mfile[1][view[0]] - 1 + numbers_key);
                        }
                    }
                }
                else
                {
                    int win_y_copy = win_y;
                    while (1)
                    {
                        if ((win_y_copy == -1) || (view[win_y] == view[win_y_copy] + 2))
                        {
                            break;
                        }
                        --win_y_copy;
                    }
                    ++win_y_copy;
                    if (mfile[1][view[win_y_copy]] == 0)
                    {
                        move(win_y_copy, numbers_key);
                    }
                    else
                    {
                        if (mfile[1][view[win_y_copy]] - 1 >= win_x)
                        {
                            move(win_y_copy, win_x);
                        }
                        else
                        {
                            move(win_y_copy, mfile[1][view[win_y_copy]] - 1 + numbers_key);
                        }
                    }
                }
            }
        }
        if (key == KEY_LEFT)
        {
            getyx(stdscr, win_y, win_x);
            if (win_x == numbers_key)
            {
                if (view[win_y - 1] == view[win_y])
                {
                    move(win_y - 1, size.ws_col - 1);
                }
                else
                {
                    if (mfile[1][view[win_y - 1]] == 0)
                    {
                        move(win_y - 1, numbers_key);
                    }
                    else
                    {
                        if (mode == 0)
                        {
                            move(win_y - 1, mfile[1][view[win_y - 1]] % size.ws_col - 1 + numbers_key);
                        }
                        else
                        {
                            move(win_y - 1, mfile[1][view[win_y - 1]] % size.ws_col + numbers_key);
                        }
                    }
                }
            }
            else
            {
                move(win_y, win_x - 1);
            }
        }
        if (key == KEY_RIGHT)
        {
            getyx(stdscr, win_y, win_x);
            if (view[win_y + 1] != -1)
            {
                if (view[win_y + 1] == view[win_y] + 1) //      -    
                {
                    if (mode == 0)
                    {
                        if (win_x - numbers_key < mfile[1][view[win_y]] % (size.ws_col - numbers_key) - 1)
                        {
                            move(win_y, win_x + 1);
                        }
                        else
                        {
                            move(win_y + 1, numbers_key);
                        }
                    }
                    else
                    {
                        if (win_x - numbers_key < mfile[1][view[win_y]] % (size.ws_col - numbers_key))
                        {
                            move(win_y, win_x + 1);
                        }
                        else
                        {
                            move(win_y + 1, numbers_key);
                        }
                    }
                }
                if (view[win_y + 1] == view[win_y]) //      -     
                {
                    if (win_x < size.ws_col - 1)
                    {
                        move(win_y, win_x + 1);
                    }
                    else
                    {
                        move(win_y + 1, numbers_key);
                    }
                }
            }
            else
            {
                if (win_x + 1 - numbers_key <= mfile[1][view[win_y]] - (mode == 0 ? 1 : 0))
                {
                    move(win_y, win_x + 1);
                }
            }
        }
        if (((key == 9) || ((key >= 32) && (key <= 126))) && (mode == 1))
        {
            getyx(stdscr, win_y, win_x);
            int counter_file = 0;
            int counter_string;
            list_p* file_copy = list_file;
            list* string_copy;
            if (file_copy == NULL)
            {
                list_file = (list_p*) malloc(sizeof(list_p));
                list_file->next = NULL;
                list_file->val = (list*) malloc(sizeof(list));
                list_file->val->next = NULL;
                list_file->val->val = key;
                print_file(win_beg_y, list_file, view);
                move(win_y, win_x + 1);
                ++mfile[0][0];
                mfile[1] = (int*) malloc(sizeof(int));
                mfile[1][0] = 1;
            }
            while(file_copy != NULL)
            {
                string_copy = file_copy->val;
                if (counter_file == view[win_y])
                {
                    counter_string = 0;
                    int win_y_copy = win_y;
                    while ((view[win_y] == view[win_y_copy]) && (win_y_copy >= 0))
                    {
                        --win_y_copy;
                    }
                    ++win_y_copy;
                    if ((win_y != win_y_copy) || (win_x != numbers_key))
                    {
                        while (string_copy != NULL)
                        { 
                            if (counter_string == win_x - 1 + (size.ws_col - numbers_key) * (win_y - win_y_copy) - numbers_key)
                            {
                                if (key != 9)
                                {
                                    string_copy->next = plus_next(string_copy, key);
                                    ++mfile[1][view[win_y]];
                                }
                                else
                                {
                                    for (int i = 0; i < tab_col; i++)
                                    {
                                        string_copy->next = plus_next(string_copy, ' ');
                                    }
                                    mfile[1][view[win_y]] += tab_col;
                                }
                                //++mfile[1][view[win_y]];
                                //print_file(win_beg_y, list_file, view);
                                if (win_x < size.ws_col - 1 - numbers_key - (key == 9 ? tab_col : 0))
                                {
                                    //move(win_y, win_x + 1);
                                    print_file(win_beg_y, list_file, view);
                                    if (key != 9)
                                    {
                                        move(win_y, win_x + 1);
                                    }
                                    else
                                    {
                                        move(win_y, win_x + tab_col);
                                    }
                                }
                                else
                                {
                                    if (win_y == size.ws_row - 2)
                                    {
                                        print_file(win_beg_y + 1, list_file, view);
                                        ++win_beg_y;
                                        move(win_y, numbers_key + (key == 9 ? tab_col - size.ws_col + win_x: 0));
                                    }
                                    else
                                    {
                                        print_file(win_beg_y, list_file, view);
                                        move(win_y + 1, numbers_key + (key == 9 ? tab_col - size.ws_col + win_x: 0));
                                    }
                                }
                                break;
                            }
                            ++counter_string;
                            string_copy = string_copy->next;
                        }
                    }
                    if ((win_x == numbers_key) && (win_y == win_y_copy))
                    {
                        if (key != 9)
                        {
                            list* new = (list*) malloc(sizeof(list));
                            new->next = string_copy;
                            new->val = key;
                            file_copy->val = new;
                            print_file(win_beg_y, list_file, view);
                            move(win_y, win_x + 1 + numbers_key);
                            ++mfile[1][view[win_y]];
                        }
                        else
                        {
                            list* new;
                            for (int i = 0; i < tab_col; i++)
                            {
                                new = (list*) malloc(sizeof(list));
                                new->next = string_copy;
                                string_copy = new;
                                new->val = ' ';
                                file_copy->val = new;
                                print_file(win_beg_y, list_file, view);
                                move(win_y, numbers_key + tab_col);
                                mfile[1][view[win_y]] += tab_col;
                            }
                        }
                    }
                    break;
                }
                file_copy = file_copy->next;
                ++counter_file;
            }
        }
        if (key == 393)
        {
            getyx(stdscr, win_y, win_x);
            int win_y_copy = win_y;
            while ((view[win_y_copy] == view[win_y]) && (win_y_copy >= 0))
            {
                --win_y_copy;
            }
            ++win_y_copy;
            move(win_y_copy, 0);
        }
        if (key == 402)
        {
            getyx(stdscr, win_y, win_x);
            int win_y_copy = win_y;
            while ((view[win_y_copy] == view[win_y]) && (win_y_copy <= size.ws_row - 2))
            {
                ++win_y_copy;
            }
            --win_y_copy;
            //printw("I AM HERE %d",win_y_copy);
            move(win_y_copy, mfile[1][view[win_y_copy]] % (size.ws_col -  numbers_key) - 1);
        }
        if (key == KEY_F(1))
        {
            win_beg_y = 0;
            print_file(win_beg_y, list_file, view);
            move(0, numbers_key);
        }
        if (key == KEY_F(2))
        {
            win_beg_y = mfile[0][0] - size.ws_row - 2;
            print_file(win_beg_y, list_file, view);
            if (mfile[1][mfile[0][0] - 1] != 0)
            {
                move (((size.ws_row - 1 > mfile[0][0]) ? mfile[0][0] - 1 : size.ws_row - 2), mfile[1][mfile[0][0] - 1] % (size.ws_col - numbers_key) - 1);
            }
            else
            {
                move (((size.ws_row - 1 > mfile[0][0]) ? mfile[0][0] - 1 : size.ws_row - 2), 0);
            }
        }
        if ((key == 127) && (mode == 1)) // backspace
        {
            getyx(stdscr, win_y, win_x);
            int counter_file = 0;
            int counter_string;
            list_p* file_copy = list_file;
            list_p* file_copy_prev = NULL;
            list* string_copy;
            list* will_free;
            while(file_copy != NULL) //   
            {
                string_copy = file_copy->val;
                if (counter_file == view[win_y]) // 
                {
                    counter_string = 0;
                    int str_beg = win_y;
                    while ((view[win_y] == view[str_beg]) && (str_beg >= 0)) //    ,      
                    {
                        --str_beg;
                    }
                    ++str_beg;
                    //if (((win_x != 1) || ((win_x == 1) && (str_beg != win_y))) && ((win_x != 0))
                    if ((str_beg != win_y) || ((win_x != 1 + numbers_key) && (win_x != 0 + numbers_key)))
                    {
                        while (string_copy != NULL)
                        {
                            if (counter_string == win_x - 2 + size.ws_col * (win_y - str_beg) - numbers_key)
                            {
                                will_free = string_copy->next;
                                string_copy->next = string_copy->next->next;
                                free(will_free);
                                --mfile[1][view[win_y]];
                                print_file(win_beg_y, list_file, view);
                                if (win_x == numbers_key)
                                {
                                    move(win_y - 1, size.ws_col - 1);
                                }
                                else
                                {
                                    move(win_y, win_x - 1);
                                }
                                break;
                            }
                            ++counter_string;
                            string_copy = string_copy->next;
                        }
                    }
                    if ((win_x == 1 + numbers_key) && (str_beg == win_y))
                    {
                        list* will_free;
                        will_free = file_copy->val;
                        file_copy->val = file_copy->val->next;
                        free(will_free);
                        --mfile[1][view[win_y]];
                        print_file(win_beg_y, list_file, view);
                        move(win_y, numbers_key);
                    }
                    if ((win_x == 0 + numbers_key) && (str_beg == win_y))
                    {
                        if (file_copy_prev != NULL)
                        {
                            list* del_str_copy = file_copy->val;
                            list_p* str_will_free = file_copy;
                            file_copy_prev->next = file_copy->next;
                            free(str_will_free);
                            list* prev_str = file_copy_prev->val;
                            if (prev_str != NULL)
                            {
                                while (prev_str->next != NULL)
                                {
                                    prev_str = prev_str->next;
                                }
                                prev_str->next = del_str_copy;
                            }
                            else
                            {
                                file_copy_prev->val = del_str_copy;
                            }
                            --mfile[0][0];
                            int for_move = mfile[1][view[win_y] - 1];
                            mfile[1][view[win_y] - 1] += mfile[1][view[win_y]];
                            for (int i = view[win_y]; i < mfile[0][0]; i++)
                            {
                                mfile[1][i] = mfile[1][i + 1];
                            }
                            mfile[1] = (int*) realloc(mfile[1], sizeof(int) * mfile[0][0] - 1);
                            if (win_y != 0)
                            {
                                print_file(win_beg_y, list_file, view);
                                move(win_y - 1, for_move % (size.ws_col - numbers_key) + numbers_key);
                            }
                            else
                            {
                                print_file(win_beg_y - 1, list_file, view);
                                --win_beg_y;
                                move(0, for_move - numbers_key);
                            }
                        }
                    }
                    break;
                }
                file_copy_prev = file_copy;
                file_copy = file_copy->next;
                ++counter_file;
            }
        }
        if ((key == 10) && (mode == 1)) // ENTER
        {
            getyx(stdscr, win_y, win_x);
            int counter_file = 0;
            int counter_string;
            list_p* file_copy = list_file;
            //list_p* file_copy_prev = NULL;
            list* string_copy;
            list* will_free;
            while(file_copy != NULL) //   
            {
                string_copy = file_copy->val;
                if (counter_file == view[win_y]) // 
                {
                    counter_string = 0;
                    int str_beg = win_y;
                    while ((view[win_y] == view[str_beg]) && (str_beg >= 0)) //    ,      
                    {
                        --str_beg;
                    }
                    ++str_beg;
                    while (string_copy != NULL)
                    {
                        if ((counter_string == win_x - 1 + size.ws_col * (win_y - str_beg) - numbers_key) || (win_x == numbers_key))
                        {
                            break;
                        }
                        ++counter_string;
                        string_copy = string_copy->next;
                    }
                    list* con = NULL;
                    if (string_copy != NULL)
                    {
                        if (win_x != numbers_key)
                        {
                            con = string_copy->next;
                            string_copy->next = NULL;
                        }
                        else
                        {
                            con = string_copy;
                            file_copy->val = NULL;
                        }
                    }
                    file_copy->next = plus_next_p(file_copy, con);
                    ++mfile[0][0];
                    mfile[1]= (int*) realloc(mfile[1], sizeof(int) * mfile[0][0]);
                    for (int i = mfile[0][0] - 1; i >= view[win_y] + 2; i--)
                    {
                        mfile[1][i] = mfile[1][i - 1];
                    }
                    mfile[1][view[win_y] + 1] = mfile[1][view[win_y]] - counter_string - ((win_x == numbers_key) ? 0 : 1);
                    mfile[1][view[win_y]] = counter_string + ((win_x == numbers_key) ? 0 : 1);
                    if (win_y != size.ws_row - 2)
                    {
                        print_file(win_beg_y, list_file, view);
                        move(win_y + 1, numbers_key);
                    }
                    else
                    {
                        print_file(win_beg_y + 1, list_file, view);
                        ++win_beg_y;
                        move(win_y, numbers_key);
                    }
                    break;
                }
                //file_copy_prev = file_copy;
                file_copy = file_copy->next;
                ++counter_file;
            }
        }
        if (key == 353)// shift + tab
        {
            getyx(stdscr, win_y, win_x);
            echo();
            move(size.ws_row - 1, 0);
            attron(COLOR_PAIR(1));
            printw("enter command:       ");
            attroff(COLOR_PAIR(1));
            //attron(COLOR_PAIR(0));
            move(size.ws_row -1, 15);
            key = getch();
            int read_key = 0;
            //printw("%d", key);
            //getch();
            // q - quit
            // s - save
            // e - start edit mode = 1 
            // r - start read mode = 0
            // n - set numbers
            if (key == 'q')
            {
                read_key = 1;
                if (save_key == 0)
                {
                    attron(COLOR_PAIR(2));
                    attron(A_BLINK);
                    move(size.ws_row - 1 , 0);
                    printw("YOU DIDN'T SAVE CHANGES!");
                    attroff(COLOR_PAIR(2));
                    attroff(A_BLINK);
                    attron(COLOR_PAIR(3));
                    printw(" press any key");
                    for (int i = 28; i < size.ws_col; i++)
                    {
                        printw(" ");
                    }
                    attroff(COLOR_PAIR(3));
                    attron(COLOR_PAIR(1));
                    getch();
                    move(size.ws_row - 1, 0);
                    printw("***** read mode *****");
                    for (int i = 21; i < size.ws_col; i++)
                    {
                        printw(" ");
                    }
                    mode = 0;
                    attroff(COLOR_PAIR(1));
                    move(win_x, win_y);
                }
                else
                {
                    free(mfile[0]);
                    free(mfile[1]);
                    free(mfile);
                    close(fd);
                    close(er);
                    delete_p(list_file);
                    free(view);
                    break;
                }
            }
            if (key == 'e')
            {
                read_key = 1;
                mode = 1;
                move(size.ws_row - 1, 0);
                print_file(win_beg_y, list_file, view);
                if (win_y != size.ws_row - 1)
                {
                    move(win_y, win_x);
                }
                else
                {
                    move(0, numbers_key);
                }
                save_key = 0;
            }
            if (key == 'n')
            {
                read_key = 1;
                numbers_key = 5;
                print_file(win_beg_y, list_file, view);
                move(win_y, win_x + numbers_key);
            }
            if (key == 'r')
            {
                read_key = 1;
                move(size.ws_row - 1, 0);
                //printw("***** read mode *****");
                print_file(win_beg_y, list_file, view);
                if (win_y != size.ws_row - 1)
                {
                    move(win_y, win_x);
                }
                else
                {
                    move(0, numbers_key);
                }
                mode = 0;
            }
            if (key == 's')
            {
                read_key = 1;
                save_key  = 1;
                close(fd);
                fd = open(argv[1], O_WRONLY | O_CREAT,0666);
                list_p* file_copy = list_file;
                list* string_copy;
                while(file_copy != NULL)
                {
                    string_copy = file_copy->val;
                    while(string_copy != NULL)
                    {
                        write(fd, (void*) &string_copy->val, sizeof(char));
                        string_copy = string_copy->next;
                    }
                    char space = '\n';
                    write(fd, &space, sizeof(char));
                    file_copy = file_copy->next;
                }
                move(size.ws_row - 1, 0);
                attron(COLOR_PAIR(1));
                printw("saved                ");
                attroff(COLOR_PAIR(1));
                move(0, numbers_key);
            }
            if (read_key == 0)
            {
                move(size.ws_row - 1, 0);
                attron(COLOR_PAIR(1));
                printw("command not found     press any key");
                getch();
                attroff(COLOR_PAIR(1));
                print_file(win_beg_y, list_file, view);
                move(win_y, win_x);
            }
        }
    }
    endwin();
    return 0;
}





