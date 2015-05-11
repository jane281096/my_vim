#include <curses.h>
#include <termios.h>
#include <sys/ioctl.h>
#include <signal.h>
#include <stdlib.h>
#include <fcntl.h>
#include <string.h>
#include <unistd.h>

int mode;
// 0 - read mode
// 1 - edit mode

struct list
{
    char val;
    struct list* next;
};

typedef struct list list;

list* plus_next(list* beg, char c)
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
            free(beg);
            if (copy == NULL)
            {
                break;
            }
            beg = copy;
        }   
    }   
    else
    {   
        //printf("in delete beg is NULL\n");
    }
} 
void sig_winch(int signo)
{
    struct winsize size;
    ioctl(fileno(stdout), TIOCGWINSZ, (char *) &size);
    resizeterm(size.ws_row, size.ws_col);
}

list_p* init_file(int fd, int** mfile)
{
    char stream;
    int er_read = read(fd, &stream, 1);
    int first_key = 1;
    //char string[1024];
    //int counter_string = 0;
    
    list_p* res = NULL;
    list_p* res_copy = NULL;
    list* res_val_copy;
    mfile[0][0] = 0;
    while (er_read != 0)
    {
        //printw("catched %c\n", stream);
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
            //printf("hello mfile\n");
            mfile[0][0] = mfile[0][0] + 1;
            //printf("goodbye mfile\n");
        }
        if ((stream >= 32) && (stream <= 126))
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
        er_read = read(fd, &stream, 1);
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

/*void print_file(int y_beg, int x_beg, list_p* list_file)
{
    struct winsize size;
    ioctl(fileno(stdout), TIOCGWINSZ, (char*) &size);
    clear();
    move(0, 0);
    //printw("in print\n");
    int counter_file = 0;
    int counter_string;
    list_p* list_file_copy = list_file;
    list* string_copy;
    int win_y, win_x;
    if (list_file_copy == NULL)
    {
        printw("NULL\n");
    }
    while (list_file_copy != NULL)
    {
        if ((counter_file >= y_beg) && (counter_file <= size.ws_row - 3))
        {
            //printw("in string process\n");
            string_copy = list_file_copy->val;
            counter_string = 0;
            while (string_copy != NULL)
            {
                if ((counter_string >= x_beg) && (counter_string < size.ws_col))
                {
                    printw("%c", string_copy->val);
                }
                ++counter_string;
                string_copy = string_copy->next;
            }
            getyx(stdscr, win_y, win_x);
            move(win_y + 1, 0);
            //printw("move done\n");
        }
        ++counter_file;
        list_file_copy = list_file_copy->next;
    }
}*/

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
    int win_y, win_x;
    while ((list_file_copy != NULL) && (counter_y_win < size.ws_row - 1))
    {
        string_copy = list_file_copy->val;
        counter_x_win = 0;
        if (counter_str_file >= str_beg)
        {
            while (string_copy != NULL)
            {
                if (counter_x_win + 1 == size.ws_col)
                {
                    view[counter_y_win] = counter_str_file;
                    ++counter_y_win;
                    counter_x_win = 0;
                }
                else
                {
                    ++counter_x_win;
                }
                printw("%c", string_copy->val);
                string_copy = string_copy->next;
            }
            view[counter_y_win] = counter_str_file;
        }
        ++counter_str_file;
        ++counter_y_win;
        list_file_copy = list_file_copy->next;
        getyx(stdscr, win_y, win_x);
        if ((win_y < counter_y_win) && (counter_str_file >= str_beg))
        {
            printw("\n");
        }
    }
}

int main(int argc, char** argv)
{
    int er = open("/Users/evgeniatveritinova1/acos/my_vim/er", O_RDWR);
    if (er == -1)
    {
        perror("open");
    }
    if (dup2(er, fileno(stderr)) < 0)
    {
        perror("dup2");
    }
    
    initscr();
    int fd = open(argv[1], O_RDWR);
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
    list_p* list_file;
    int** mfile;
    mfile = (int**) malloc(sizeof(int*) * 2);
    mfile[0] = (int*) malloc(sizeof(int));
    mfile[1] = NULL;
    list_file = init_file(fd, mfile);
    int view[size.ws_row - 1];
    for (int i = 0; i < size.ws_row - 1; i++)
    {
        view[i] = -1;
    }
    /*for (int i = 0; i < mfile[0][0]; i++)
    {
        view[i] = (int*) malloc(sizeof(int) * ((mfile[1][i] - 1) / size.ws_col + 1));
    }*/
    print_file(0, list_file, view);
    int win_beg_y = 0;
    char command[5];
    //int counter_string = 0;
    mode = 0;
    move(0, 0);
    
    /*clear();
    move(0, 0);
    for (int i = 0; i < size.ws_row - 1; i++)
    {
        printw("%d   %d\n", i, view[i]);
    }*/


    while (1)
    {
        ioctl(fileno(stdout), TIOCGWINSZ, (char*) &size); 
        if (mode == 0)
        {
            noecho();
        }
        key = getch();
        if (key == KEY_DOWN)
        {
            getyx(stdscr, win_y, win_x);
            if (view[win_y + 1] <= mfile[0][0] - 1)
            {
                if (win_y + 1 == size.ws_row - 1)
                {
                    print_file(win_beg_y + 1, list_file, view);
                    ++win_beg_y;
                    move(win_y, win_x);
                }
                else
                {
                    if (mfile[1][view[win_y]] != 0)
                    {
                        if (win_x > mfile[1][view[win_y]] - 1)
                        {
                            move(win_y + 1, mfile[1][view[win_y]] - 1);
                        }
                        else
                        {
                            move(win_y + 1, win_x);
                        }
                    }
                    else
                    {
                        move(win_y + 1, 0);
                    }
                }
            }
        }
        if (key == KEY_UP)
        {
            getyx(stdscr, win_y, win_x);
            if (win_y == 0)
            {
                if (win_beg_y > 0)
                {
                    print_file(win_beg_y - 1, list_file, view);
                    --win_beg_y;
                    move(0, win_x);
                }
            }
            else
            {
                move(win_y - 1, win_x);
            }
        }
        /*
        if (key == KEY_LEFT)
        {
            getyx(stdscr, win_y, win_x);
            if(win_x == 0)
            {
                if (win_beg_x > 0)
                {
                    print_file(win_beg_y, win_beg_x - 1, list_file);
                    --win_beg_x;
                    move(win_y, 0);
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
            if (win_x + 1 != size.ws_col)
            {
                if (win_x < mfile[1][win_y] - 1)
                {
                    move(win_y, win_x + 1);
                }
                else
                {
                    if (win_y < mfile[0][0] - 1)
                    {
                        move(win_y + 1, 0);
                    }
                }
            }
            else
            {
                print_file(win_beg_y, win_beg_x + 1, list_file);
                ++win_beg_x;
                move(win_y, win_x);
            }
        }*/
        if (key == 353)// shift + tab
        {
            echo();
            ioctl(fileno(stdout), TIOCGWINSZ, (char*) &size);
            move(size.ws_row - 1, 0);
            key = getch();
            if (key == 'q')
            {
                for (int i = 0; i < mfile[0][0] + 2; i++)
                {
                    free(mfile[i]);
                }
                free(mfile);
                break;
            }
        }
    }
    //getch();
    endwin();
    return 0;
}

