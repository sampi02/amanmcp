#include<stdio.h>
void main()
{
    int n,i;
    int m;
    printf("enter the number of students\n");
    scanf("%d",&m);
    printf("enter the number of modules\n");
    scanf("%d",&n);
    int marks[100];
    for(int j=0;j<m;j++)
    {
    for(i=0;i<n;i++)
    {
    printf("Enter your marks ");
    scanf("%d",&marks[i]);
    }
        printf("The student %d grade is: "\n);
    for(i=0;i<n;i++)
    {
    if(marks[i]<0 || marks[i]>100)
    {
        printf("Wrong Entry\n");
    }
    else if(marks[i]<50)
    {
        printf("Grade F\n");
    }
    else if(marks[i]>=50 && marks[i]<60)
    {
        printf("Grade D\n");
    }
    else if(marks[i]>=60 && marks[i]<70)
    {
        printf("Grade C\n");
    }
    else if(marks[i]>=70 && marks[i]<80)
    {
        printf("Grade B\n");
    }
    else if(marks[i]>=80 && marks[i]<90)
    {
        printf("Grade A\n");
    }
    else
    {
        printf("Grade A+\n");
    }
    }
}




