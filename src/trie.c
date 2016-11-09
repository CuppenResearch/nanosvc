/* BioCell - Cellular base for computational biology.
 * Copyright © 2015  Roel Janssen <roel@gnu.org>
 *
 * This file is part of BioCell.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "trie.h"

#include <string.h>
#include <stdlib.h>

struct trie_node_t *
trie_new (void)
{
  return calloc (1, sizeof (struct trie_node_t));
}

static struct trie_node_t *
trie_create_path (const char *key, void *element)
{
  if (key == NULL || element == NULL)
    return NULL;

  struct trie_node_t *node = trie_new ();
  if (node == NULL)
    return NULL;

  node->key = key[0];

  if (strlen (key) > 1)
    {
      struct trie_node_t **children;
      struct trie_node_t *child;
      child = trie_create_path (key + 1, element);
      children = realloc (node->children, sizeof (struct trie_node_t *) *
                                          (node->children_len + 1));

      if (children == NULL)
        return NULL;
      else
        {
          children[node->children_len] = child;
          node->children = children;
          node->children_len++;
        }
    }
  else
      node->element = element;

  return node;
}

bool
trie_insert (struct trie_node_t *trie, const char *name, void *element)
{
  if (trie == NULL || element == NULL)
    return false;

  size_t name_len = strlen (name);
  size_t position = 0;
  struct trie_node_t *node = trie;

  size_t child_index = 0;
  struct trie_node_t *child;

  while (position < name_len && child_index < node->children_len)
    {
      child = node->children[child_index];
      if (child->key == name[position])
        {
          node = child;
          child_index = 0;
          position++;
        }
      else
        child_index++;
    }

  if (position == name_len)
    node->element = element;
  else if (position < name_len)
    {
      struct trie_node_t *branch;
      branch = trie_create_path (name + position, element);
      if (branch == NULL)
        return false;

      struct trie_node_t **children;
      children = realloc (node->children, sizeof (struct trie_node_t *) *
                                          (node->children_len + 1));

      if (children == NULL)
        return false;

      children[node->children_len] = branch;
      node->children = children;
      node->children_len++;
    }

  return true;
}

void *
trie_find (struct trie_node_t *trie, const char *name)
{
  if (trie == NULL || name == NULL)
    return NULL;

  size_t name_len = strlen (name);
  size_t position = 0;
  struct trie_node_t *node = trie;

  size_t child_index = 0;
  struct trie_node_t *child = NULL;

  while (position < name_len && child_index < node->children_len)
    {
      child = node->children[child_index];
      if (child->key == name[position])
        {
          node = child;
          child_index = 0;
          position++;
        }
      else
        child_index++;
    }

  if (position == name_len)
    return child->element;

  return NULL;
}

uint32_t
trie_elements_in_trie (struct trie_node_t *trie)
{
  if (trie == NULL)
    return 0;

  uint32_t num_elements = 0;

  size_t index;
  for (index = 0; index < trie->children_len; index++)
    num_elements += trie_elements_in_trie (trie->children[index]);

  if (trie->element != NULL)
    num_elements++;

  return num_elements;
}

void
trie_destroy (struct trie_node_t *trie)
{
  if (trie == NULL)
    return;

  size_t index;
  for (index = 0; index < trie->children_len; index++)
    trie_destroy (trie->children[index]);

  trie->key = '\0';
  trie->element = NULL;

  free (trie->children);
  trie->children_len = 0;

  free (trie);
}

void
trie_destroy_full (struct trie_node_t *trie, void (*callback) (void *))
{
  if (trie == NULL)
    return;

  size_t index;
  for (index = 0; index < trie->children_len; index++)
    trie_destroy_full (trie->children[index], callback);

  trie->key = '\0';

  if (trie->element != NULL)
    callback (trie->element);
  trie->element = NULL;

  free (trie->children);
  trie->children_len = 0;

  free (trie);
}
