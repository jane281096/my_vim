#include <math.h>
#include <curses.h>
#include <termios.h>
#include <sys/ioctl.h>
#include <signal.h>
#include <stdlib.h>
#include <fcntl.h>
#include <string.h>
#include <unistd.h>

int mode = 0;
int numbers_key = 0;

struct list
{
    unsigned char val;
    struct list* next;
};

typedef struct list list;

list* plus_next(list* beg, unsigned char c)
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
    unsigned char stream;
    int er_read = read(fd, &stream, sizeof(unsigned char));
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
        er_read = read(fd, &stream, sizeof(unsigned char));
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
    move(0, numbers_key);

    while (1)
    {
        noecho();
        ioctl(fileno(stdout), TIOCGWINSZ, (char*) &size); 
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
                    if (mfile[1][view[size.ws_row - 2]] == 0)
                    {
                        move(size.ws_row - 2, numbers_key);
                    }
                    else
                    {
                        if (mfile[1][view[size.ws_row - 2]] > size.ws_col) // если текущая строка не влезает в экран
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
                        move(win_y_copy, numbers_key);
                    }
                    else
                    {
                        if (mfile[1][view[win_y_copy]] - 1 >= win_x) // если при перемещении курсора по вертикали, мы попадем на строчку
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
        if (key == KEY_UP) // переходит к предыдущей строчке в файле, а не на экране6
        {
            getyx(stdscr, win_y, win_x);
            if (view[win_y] - 1 >= 0) // если мы не на первой строчке фалйа
            {
                if (win_y == 0) // если текущая строчка в начале экрана
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
                if (view[win_y + 1] == view[win_y] + 1) // если следующая строчка на экране - следующая строчка в файле
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
                if (view[win_y + 1] == view[win_y]) // если следующая строчка на экране - продолжение текущей строчки в файле
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
        if ((key >= 32) && (key <= 255) && (mode == 1) && (key != 127))
        {
            getyx(stdscr, win_y, win_x);
            int counter_file = 0;
            int counter_string;
            list_p* file_copy = list_file;
            list* string_copy;
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
                                string_copy->next = plus_next(string_copy, key);
                                ++mfile[1][view[win_y]];
                                //print_file(win_beg_y, list_file, view);
                                if (win_x != size.ws_col - 1 - numbers_key)
                                {
                                    //move(win_y, win_x + 1);
                                    print_file(win_beg_y, list_file, view);
                                    move(win_y, win_x + 1);
                                }
                                else
                                {
                                    if (win_y == size.ws_row - 2)
                                    {
                                        print_file(win_beg_y + 1, list_file, view);
                                        ++win_beg_y;
                                        move(win_y, numbers_key);
                                    }
                                    else
                                    {
                                        print_file(win_beg_y, list_file, view);
                                        move(win_y + 1, numbers_key);
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
                        list* new = (list*) malloc(sizeof(list));
                        new->next = string_copy;
                        new->val = key;
                        file_copy->val = new;
                        print_file(win_beg_y, list_file, view);
                        move(win_y, win_x + 1 + numbers_key);
                        ++mfile[1][view[win_y]];
                    }
                    break;
                }
                file_copy = file_copy->next;
                ++counter_file;
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
            while(file_copy != NULL) // находим нужную строку
            {
                string_copy = file_copy->val;
                if (counter_file == view[win_y]) // нашли
                {
                    counter_string = 0;
                    int str_beg = win_y;
                    while ((view[win_y] == view[str_beg]) && (str_beg >= 0)) // находим строчку на экране, на которой начинается текущая строчка файла
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
            while(file_copy != NULL) // находим нужную строку
            {
                string_copy = file_copy->val;
                if (counter_file == view[win_y]) // нашли
                {
                    counter_string = 0;
                    int str_beg = win_y;
                    while ((view[win_y] == view[str_beg]) && (str_beg >= 0)) // находим строчку на экране, на которой начинается текущая строчка файла
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
            move(size.ws_row -1, 15);
            key = getch();
            //printw("%d", key);
            // q - quit
            // s - save
            // e - start edit mode = 1 
            // r - start read mode = 0
            // n - set numbers
            if (key == 'q')
            {
                free(mfile[0]);
                free(mfile[1]);
                free(mfile);
                break;
            }
            if (key == 'e')
            {
                mode = 1;
                move(size.ws_row - 1, 0);
                printw("***** edit mode *****");
                if (win_y != size.ws_row - 1)
                {
                    move(win_y, win_x);
                }
                else
                {
                    move(0, numbers_key);
                }
            }
            if (key == 'n')
            {
                numbers_key = 5;
                print_file(win_beg_y, list_file, view);
                move(win_y, win_x + numbers_key);
            }
            if (key == 'r')
            {
                move(size.ws_row - 1, 0);
                printw("***** read mode *****");
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
        }
    }
    endwin();
    return 0;
}

