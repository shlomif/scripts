/* https://en.wikipedia.org/wiki/Segmentation_fault */
int main()
{
    char *s = "hello world";
    *s = 'H';
}
