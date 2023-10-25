#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
FILE *my_open(char *name, char *mode)
{
	FILE *file = fopen(name, mode);

	if (!file)
	{
		fprintf(stderr, "Failed to open %s\n", name);
		perror("fopen");
		exit(1);
	}

	return (file);
}
char test_tag(char c, char *tag, FILE *file)
{
	unsigned long int count = 0;

	while (c == tag[count])
	{
		count++;
		if (tag[count] == '\0')
			break;
		c = fgetc(file);
		/*if (c == EOF)
			return 0;*/
	}
	if (count == strlen(tag))
		return (1);
	else if (count != 0)
		fseek(file, -count, SEEK_CUR);

	return (0);
}
char copy_between_tags(char *tag_start, char *tag_end, FILE *source, FILE *README)
{
	char c;
	int i = 0;

	/*fprintf(README, "start copy\n");*/
	while ((c = fgetc(source)) != EOF)
	{
		if (test_tag(c, tag_start, source))
		{
			while ((c = fgetc(source)) != EOF)
			{
				if (c == '>')
					break;
			}
			while ((c = fgetc(source)) != EOF)
			{
				if (c != ' ' && c != '\n')
				{
					fprintf(README, "%c", c);
					break;
				}
			}
			while ((c = fgetc(source)) != EOF)
			{
				if (test_tag(c, tag_end, source))
					break;
				fprintf(README, "%c", c);
			}
			return (1);
			/*fprintf(README, "end tag\n");*/
		}
	}
	return (0);
}
void print_title(FILE *source, FILE *README)
{
	fprintf(README, "# ");
	copy_between_tags("<h1 style=", "</h1>", source, README);
	fprintf(README, "\n");
}
char print_next_task(FILE *source, FILE *README)
{
	if (!copy_between_tags("<div class=\"panel-heading panel-heading-actions\"", "<div>", source, README))
		return (0);
	copy_between_tags("<!-- Task Body -->", "<!-- Self-paced manual review -->", source, README);
	return (1);
}
void clean_README()
{
	FILE *README, *copy;
	char c;
	int leadingSpaces = 1;

	system("touch files/copy.txt");
	README = my_open("output/README.md", "r");
	copy = my_open("output/copy.txt", "w+");
	while ((c = fgetc(README)) != EOF) {
		if (leadingSpaces) {
			if (c == ' ' || c == '\t') {
				continue; // Skip leading spaces
			} else {
				leadingSpaces = 0; // No more leading spaces
			}
		}
		fprintf(copy, "%c", c);
		if (c == '\n') {
			leadingSpaces = 1; // Reset the flag for the next line
		}
	}
	/*while ((c = fgetc(README)) != EOF)
	{
		if (c == '\n')
		{
			fprintf(copy, "%c", c);
			while ((c = fgetc(README)) != EOF)
			{

				fprintf(copy, "%d", c);
				if (c != ' ' && c != '\t')
				{
					fprintf(copy, "%c", c);
					break;
				}
			}
		}
		else
			fprintf(copy, "%c", c);
	}*/

	fclose(README);
	fclose(copy);

	system("mv output/copy.txt output/README.md");
}
int main(void)
{
	FILE *source, *README;

	source = my_open("files/class_containing_tasks.html", "r");
	README = my_open("output/README.md", "w+");

	print_title(source, README);
	fprintf(README, "\n");


	fseek(source, 0, SEEK_SET);
	copy_between_tags("<div class=\"panel panel-default\" id=\"project-description\">\n", "</div>", source, README);
	/*copy_between_tags("<div class=\"panel-body\">", "</div>", source, README);*/
	fprintf(README, "\n<details>\n<summary>");
	fprintf(README, "Click to see: Tasks");
	fprintf(README, "</summary>\n\n");

	while (print_next_task(source, README))
		continue;

	/*copy_between_tags("<div data-role=", "</div>", source, README);*/

	fprintf(README, "</details>\n");

	fclose(source);
	fclose(README);

	clean_README();
}
