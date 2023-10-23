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
	char *name = NULL, *def = NULL;

	if (file_name[strlen(file_name) - 1] == '/')
		return;
	name = malloc(strlen("output/") + strlen(file_name) + 1);

	if (name == NULL)
		exit(1);

	name[0] = '\0';
	strcat(name, "output/");
	strcat(name, file_name);
	strcat(name, "\0");

	file = my_open(name, "w+");
	def = get_str(" <code>def", "</code>");
	if (strlen(file_name) > 2)
		if (file_name[strlen(file_name)-2] == 'p' && file_name[strlen(file_name)-1] == 'y')
		{
			fprintf(file, "#!/usr/bin/python3\n");
			if (def != NULL)
				fprintf(file,"def%s\n", def);
		}

	free(def);
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
	if (file_name != NULL)
	{
		detect_double_files(file_name);

	}

	free(file_name);

}
char *find_user_name(FILE *task)
{
	char c, user[50], *username;
	int i = 0;

	while ((c = fgetc(task)) != EOF)
	{
		if (test_tag(c, "<pre><code>", task))
		{
			while ((c = fgetc(task)) != EOF)
			{
				if (c == '\n')
					return (NULL);
				if (test_tag(c, "$ cat", task))
					break;
				user[i] = c;
				i++;
			}
			user[i] = '\0';
			username = malloc(strlen(user) + 1);
			strcpy(username, user);
			username[i] = '\0';
			return (username);
		}
	}
	return (NULL);
}
void create_file_print_code(char *file_name, char *username, FILE *task)
{
	FILE *file;
	char *name = malloc(strlen("output/") + strlen(file_name) + 1), c;

	if (name == NULL)
		exit(1);

	name[0] = '\0';
	strcat(name, "output/");
	strcat(name, file_name);
	strcat(name, "\0");
	file = my_open(name, "w+");
	free(name);


	while ((c = fgetc(task)) != EOF)
	{
		if (test_tag(c, username, task))
			break;
		fprintf(file, "%c", c);
	}

	fclose(file);
}
void get_all_cats()
{
	FILE *task = my_open("files/task.txt", "r");
	char c, name[200], *str = NULL;
	int i = 0;

	str = find_user_name(task);
	if (str == NULL)
		return;
	/*printf("user: %s\n", str);*/
	fseek(task, -5, SEEK_CUR);
	while ((c = fgetc(task)) != EOF)
	{
		if (test_tag(c, "$ cat ", task))
		{
			while ((c = fgetc(task)) != EOF)
			{
				if (c == '\n')
					break;
				name[i] = c;
				i++;
			}
			name[i] = '\0';
			i = 0;

			/*printf("file: %s\n", name);*/
			create_file_print_code(name, str, task);
		}
	}
	free(str);
	fclose(task);
}
void clean_task()
{
	FILE *task, *copy;
	char c;

	system("touch files/copy.txt");
	task = my_open("files/task.txt", "r");
	copy = my_open("files/copy.txt", "w+");

	while ((c = fgetc(task)) != EOF)
	{
		/*printf("%d ", c);*/
		if (test_tag(c, "&gt;", task))
			fprintf(copy, ">");
		else
			fprintf(copy, "%c", c);
	}

	fclose(task);
	fclose(copy);

	system("mv files/copy.txt files/task.txt");
}
void clean_html()
{
	FILE *task, *copy;
	char c;

	system("touch files/copy.txt");
	task = my_open("files/class_containing_tasks.html", "r");
	copy = my_open("files/copy.txt", "w+");

	while ((c = fgetc(task)) != EOF)
	{
		if (test_tag(c, "&gt;", task))
			fprintf(copy, "%c", '>');
		else if (test_tag(c, "&amp;", task))
			fprintf(copy, "%c", '&');
		else if (test_tag(c, "&quot;", task))
			fprintf(copy, "%c", '"');
		else if (test_tag(c, "&lt;", task))
			fprintf(copy, "%c", '<');
		else if (test_tag(c, "&#39;", task))
			fprintf(copy, "%c", '\'');
		else if (test_tag(c, "&ldquo;", task))
			fprintf(copy, "%c", '"');
		else if (test_tag(c, "&rdquo;", task))
			fprintf(copy, "%c", '"');
		else if (test_tag(c, "&times;", task))
			fprintf(copy, "%c", '*');
		else if (test_tag(c, "&rsquo;", task))
			fprintf(copy, "%c", '\'');
		else
			fprintf(copy, "%c", c);
	}

	fclose(task);
	fclose(copy);

	system("mv files/copy.txt files/class_containing_tasks.html");
}
int main(void)
{
	clean_html();

	FILE *source = my_open("files/class_containing_tasks.html", "r");
	char c;
	int i = 0;

	system("rm output/*");
	system("mkdir output");

	do {
		i++;

		c = get_task(source);
		if (c == 0)
			break;
		clean_task();

		get_all_cats();
		/*main = get_main();

		if (main != NULL)
		{
			create_main_and_print_code(main);
			free(main);
			main = NULL;
		}*/
		create_file();
	} while (1);
	system("chmod u+x output/*.py");
	fclose(source);
}
