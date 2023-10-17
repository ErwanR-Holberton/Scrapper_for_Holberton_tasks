#include <stdio.h>
#include <string.h>
#include <stdlib.h>
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
	}
	if (count == strlen(tag))
		return (1);
	else if (count != 0)
		fseek(file, -count, SEEK_CUR);

	return (0);
}
char get_task(FILE *source)
{
	FILE *task = my_open("files/task.txt", "w+");
	char c;
	while ((c = fgetc(source)) != EOF)
	{
		if (test_tag(c, "<!-- Task Body -->", source))
		{
			fprintf(task, "----------<T1>-----------\n");

			while ((c = fgetc(source)) != EOF)
			{
				if (test_tag(c, "<!-- Self-paced manual review -->", source))
				{
					fprintf(task, "----------<end T1>-----------\n");
					fclose(task);
					return (1);
				}
				fprintf(task, "%c", c);
			}
			break;
		}
	}
	fclose(task);
	return (0);
}
char *gpt_get_str(char *tag_start, char *tag_end)
{
	FILE *task = my_open("files/task.txt", "r");
	char c;
	int i = 0;
	char *str = NULL;
	int inTag = 0;

	printf("start gpt\n");

	while ((c = fgetc(task)) != EOF)
	{
		printf("loop\n");
		if (inTag)
		{
			if (test_tag(c, tag_end, task))
			{
				break;  // End tag found, exit the loop
			}
		}
		else if (test_tag(c, tag_start, task))
		{
			inTag = 1;  // Start tag found, start reading content
		}
		else if (inTag)
		{
			if (i == 0)
			{
				str = malloc(200);  // Allocate initial buffer
				if (str == NULL)
				{
					perror("malloc");
					exit(1);
				}
			}
			str[i++] = c;

			// Check if the buffer is full, and if so, reallocate it
			if (i % 200 == 0)
			{
				str = realloc(str, i + 200);
				if (str == NULL)
				{
					perror("realloc");
					exit(1);
				}
			}
		}
	}

	if (str != NULL)
	{
		str[i] = '\0';  // Null-terminate the string
	}

	fclose(task);
	return str;  // Return the string (or NULL if not found)
}
char *get_str(char *tag_start, char *tag_end)
{
	FILE *task = my_open("files/task.txt", "r");
	char c, main[200], *str = NULL;
	int i = 0;

	while ((c = fgetc(task)) != EOF)
	{
		if (test_tag(c, tag_start, task))
		{
			while ((c = fgetc(task)) != EOF)
			{
				if (test_tag(c, tag_end, task))
					break;
				main[i] = c;
				i++;
			}
			main[i] = '\0';
			str = malloc(strlen(main) + 1);
			strcpy(str, main);
			str[i] = '\0';
			fclose(task);
			return (str);
		}
	}
	fclose(task);
	return (NULL);
}
char *get_main()
{
	return (get_str("$ cat ", "\n"));
}
char create_main_and_print_code(char *main)
{
	char *path = get_str("<pre><code>", " cat ");
	FILE *task = my_open("files/task.txt", "r"), *file;
	char *name = malloc(strlen("output/") + strlen(main) + 1), c;

	if (name == NULL)
		exit(1);

	name[0] = '\0';
	strcat(name, "output/");
	strcat(name, main);
	strcat(name, "\0");
	file = my_open(name, "w+");
	free(name);

	while ((c = fgetc(task)) != EOF)
	{
		if (test_tag(c, "$ cat ", task))
		{
			while ((c = fgetc(task)) != '\n')
			{
			}
			while ((c = fgetc(task)) != EOF)
			{
				if (test_tag(c, path, task))
					break;
				fprintf(file, "%c", c);
			}
			break;
		}
	}
	free(path);
	fclose(task);
	fclose(file);
	return (0);
}
void file(char *file_name)
{
	FILE *file;
	char *name = NULL;

	name = malloc(strlen("output/") + strlen(file_name) + 1);

	if (name == NULL)
		exit(1);

	name[0] = '\0';
	strcat(name, "output/");
	strcat(name, file_name);
	strcat(name, "\0");

	file = my_open(name, "w+");
	fprintf(file, "#!/usr/bin/python3\n");
	fclose(file);
	free(name);
}
void detect_double_files(char *file_name)
{
	int len = strlen(file_name), i;
	char *copy = file_name;


	for (i = 0; i < len; i++)
	{
		if (file_name[i] == ',')
		{
			file_name[i] = '\0';
			i++;
			file(copy);
			copy = &file_name[i + 1];
		}
		if (file_name[i] == '/')
		{
			copy = &file_name[i + 1];
		}
	}
	file(copy);
}
void create_file()
{
	char *file_name = NULL;

	file_name = get_str("<li>File: <code>", "</code></li>");
	detect_double_files(file_name);

}
int main(void)
{
	FILE *source = my_open("files/class_containing_tasks.html", "r");
	char *main = NULL, c;

	system("rm output/*");

	do {

		c = get_task(source);
		if (c == 0)
			break;
		main = get_main();

		if (main != NULL)
		{
			create_main_and_print_code(main);
			free(main);
			main = NULL;
		}
		create_file();
	} while (1);
	system("chmod u+x output/*.py");
	fclose(source);
}
