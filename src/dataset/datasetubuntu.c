#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <dirent.h>

#include "dataset.h"

static unsigned long next_document_id = 1;

static void add_words(InvertedIndex *index, const char *text)
{
    char word[256];
    int pos = 0;

    for (size_t i = 0;; ++i)
    {
        char c = text[i];

        if (isalnum((unsigned char)c) || c == '_')
        {
            if (pos < 255)
                word[pos++] = (char)tolower((unsigned char)c);
        }
        else
        {
            if (pos > 0)
            {
                word[pos] = '\0';

                Posting posting;
                posting.document_id = next_document_id;
                posting.frequency = 1;

                index_insert(index, word, posting);

                pos = 0;
            }

            if (c == '\0')
                break;
        }
    }
}

static void read_file(const char *path, InvertedIndex *index)
{
    FILE *file = fopen(path, "r");

    if (file == NULL)
        return;

    char buffer[1024];

    while (fgets(buffer, sizeof(buffer), file) != NULL)
    {
        add_words(index, buffer);
    }

    fclose(file);
    next_document_id++;
}

int index_dataset_folder(const char *folder, InvertedIndex *index)
{
    DIR *dir;
    struct dirent *entry;

    if (folder == NULL || index == NULL)
        return 1;

    dir = opendir(folder);

    if (dir == NULL)
    {
        fprintf(stderr, "Could not open dataset folder: %s\n", folder);
        return 1;
    }

    while ((entry = readdir(dir)) != NULL)
    {
        const char *name = entry->d_name;
        const char *extension = strrchr(name, '.');

        if (extension == NULL)
            continue;

        if (strcasecmp(extension, ".txt") == 0 ||
            strcasecmp(extension, ".json") == 0 ||
            strcasecmp(extension, ".conf") == 0)
        {
            char file_path[1024];

            snprintf(file_path, sizeof(file_path),
                     "%s/%s", folder, name);

            printf("Indexing: %s\n", name);

            read_file(file_path, index);
        }
    }

    closedir(dir);

    return 0;
}
