#include <stdio.h>

void merge(int a[], int left, int right, int mid)
{
    int temp[100];
    int i = left;
    int j = mid + 1;
    int k = left;
    // 合并两个有序子数组
    while (i <= mid && j <= right) {
        if (a[i] < a[j]) {
            temp[k++] = a[i++];
        } else {
            temp[k++] = a[j++];
        }
    }
    // 复制剩余元素
    while (i <= mid) {
        temp[k++] = a[i++];
    }
    while (j <= right) {
        temp[k++] = a[j++];
    }
    //复制数组
    for (int i = left; i <= right; i++) {
        a[i] = temp[i];
    }
}

void merge(int a[], int left, int right)
{
    if (left >= right) {
        return;
    }

    int mid = (left+right)/2;
    merge(a, left, mid);
    merge(a, mid+1, right);

    int temp[100];
    int i = left;
    int j = mid + 1;
    int k = left;
    // 合并两个有序子数组
    while (i <= mid && j <= right) {
        if (a[i] < a[j]) {
            temp[k++] = a[i++];
        } else {
            temp[k++] = a[j++];
        }
    }
    // 复制剩余元素
    while (i <= mid) {
        temp[k++] = a[i++];
    }
    while (j <= right) {
        temp[k++] = a[j++];
    }
    //复制数组
    for (int i = left; i <= right; i++) {
        a[i] = temp[i];
    }
}

void bubble(int a[], int n)
{
    for (int i = 0; i < n-1; i++) {
        for (int j = 0; j < n-1-i; j++) {
            if (a[j] > a[j+1]) {
                int t = a[j];
                a[j] = a[j+1];
                a[j+1] = t;
            }
        }
    }
}

void selection(int a[], int n)
{
    for (int i = 0; i < n; i++) {
        for (int j = i; j < n; j++) {
            if (a[i] > a[j]) {
                int t = a[i];
                a[i] = a[j];
                a[j] = t;
            }
        }
    }
}

int main()
{
    int n;
    scanf("%d", &n);
    int num[n];
    for (int i = 0; i < n; i++) {
        scanf("%d", &num[i]);
    }
    merge(num, 0, n-1);
    for (int i = 0; i < n; i++) {
        printf("%d ", num[i]);
    }

    return 0;
}