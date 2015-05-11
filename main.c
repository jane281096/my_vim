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
            string_copy = list_file_copy->val;
            counter_x_win = 0;
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
    int view[size.ws_row];
    print_file(0, list_file, view);
    int win_beg_y = 0;
    char command[5];
    mode = 0;
    move(0, 0);

    while (1)
    {
        ioctl(fileno(stdout), TIOCGWINSZ, (char*) &size); 
        if (mode == 0)
        {
            noecho();
        }
        key = getch();
        if (key == KEY_DOWN) // переходит к следующей строчке в файле, а не а экране
        {
            getyx(stdscr, win_y, win_x);
            if (view[win_y] + 1 < mfile[0][0]) // если мы не на последней строчке файла
            {
                if (win_y == size.ws_row - 2) // eсли текущая строчка в конце экрана
                {
                    print_file(win_beg_y + 1, list_file, view);
                    ++win_beg_y;
                    move(win_y, win_x);
                }
                else // если текущая строчка не в конце экрана
                {
                    // поиск номера первой строчки на экране, где записана следующая строчка файла
                    int beg = view[win_y];
                    int win_y_copy = win_y;
                    while (beg == view[win_y_copy])
                    {
                        ++win_y_copy;
                    }
                    if(mfile[1][view[win_y_copy]] == 0) // если следующая строчка пустая 
                    {
                        move(win_y_copy, 0);
                    }
                    else
                    {
                        if (mfile[1][view[win_y_copy]] - 1 >= win_x) // если при перемещении курсора по вертикали, мы попадем на строчку
                        {
                            move(win_y_copy, win_x);
                        }
                        else
                        {
                            move(win_y_copy, mfile[1][view[win_y_copy]] - 1);
                        }
                    }
                }
            }
        }
        if (key == KEY_UP) // переходит к предыдущей строчке в файле, а не на экране6
        {
            getyx(stdscr, win_y, win_x);
            if (view[win_y] - 1 >= 0) // если мы не на первой строчке фалйа
            {
                if (win_y == 0) // если текущая строчка в начале экрана
                {
                    print_file(win_beg_y - 1, list_file, view);
                    --win_beg_y;
                    move(0, win_x);
                }
                else
                {
                    int beg = view[win_y];
                    int win_y_copy = win_y;
                    while (beg == view[win_y_copy])
                    {
                        --win_y_copy;
                    }
                    if (mfile[1][view[win_y_copy]] == 0)
                    {
                        move(win_y_copy, 0);
                    }
                    else
                    {
                        if (mfile[1][view[win_y_copy]] - 1 >= win_x)
                        {
                            move(win_y_copy, win_x);
                        }
                        else
                        {
                            move(win_y_copy, mfile[1][view[win_y_copy]] - 1);
                        }
                    }
                }
            }
        }
        if (key == KEY_LEFT)
        {
            getyx(stdscr, win_y, win_x);
            move(win_y, win_x - 1);
        }
        if (key == KEY_RIGHT)
        {
            getyx(stdscr, win_y, win_x);
            if (view[win_y +1] != -1)
            {
                if (view[win_y + 1] > view[win_y]) // если следующая строчка на экране - следующая строчка в файле
                {
                    if (win_x < mfile[1][view[win_y]] - 1)
                    {
                        move(win_y, win_x + 1);
                    }
                }
                if (view[win_y + 1] == view[win_y]) // если следующая строчка на экране - продолжение текущей строчки в файле
                {
                    if (win_x < size.ws_col - 1)
                    {
                        move(win_y, win_x + 1);
                    }
                    else
                    {
                        move(win_y + 1, 0);
                    }
                }
            }
            else
            {
                if (win_x + 1 <= mfile[1][view[win_y]] - 1)
                {
                    move(win_y, win_x + 1);
                }
            }
        }
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

