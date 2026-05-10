#include <stdio.h>
#include <stdbool.h>

struct date {
    int month;
    int day;
    int year;
};

bool isLeap(struct date d);
int numberOfDays(struct date d);

int main()
{
    struct date today,tommorow;

    printf("Enter today's date (mm dd yyyy):\n");
    scanf("%i %i %i",&today.month,&today.day,&today.year);

    if (today.day != numberOfDays(today)) {
        tommorow.day = today.day + 1;
        tommorow.month = today.month;
        tommorow.year = today.year;
    } else if (today.month == 12) {
        tommorow.day = 1;
        tommorow.month = 1;
        tommorow.year = today.year + 1;
    } else {
        tommorow.day = 1;
        tommorow.month = today.month + 1;
        tommorow.year = today.year;
    }

    printf("Tommorow's date is %i-%i-%i.\n",tommorow.year,tommorow.month,tommorow.day);

    return 0;
}

bool isLeap(struct date d)
{
    bool leap = false;

    if ((d.year % 4 == 0 && d.year % 100 != 0) || d.year%400 == 0) {
        leap = true;
    }

    return leap;
}

int numberOfDays(struct date d)
{
    int days;

    const int daysPerMonth[12] = {31,28,31,30,31,30,31,30,31,31,30,31,30,31};

    if (d.month == 2 && isLeap(d)) {
        days = 29;
    } else {
        days = daysPerMonth[d.month-1];
    }

    return days;
}