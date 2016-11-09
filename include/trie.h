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

#ifndef LIBGENOME_TRIE_H_
#define LIBGENOME_TRIE_H_

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

/**
 * A trie-like structure that can find an element by its name in 
 * -length of name- steps. */
struct trie_node_t
{
  char key; /*< Should match the corresponding position of the identifier. */
  struct trie_node_t **children; /*< The options after this key. */
  size_t children_len; /*< The length of the children array.  */
  void *element; /*< The element that can be identified at this point. */
};

/**
 * Creates the root node of a trie.
 * @return A pointer to a dynamically allocated trie_node_t.
 */
struct trie_node_t * trie_new (void);

/**
 * Inserts the element in the trie.
 * @param trie     The trie to insert the element into.
 * @param name     The name to store the element under.
 * @param element  The element to insert in the trie.
 *
 * @return true when it got inserted, false otherwise.
 */
bool trie_insert (struct trie_node_t *trie, const char *name, void *element);

/**
 * Searches for an element in the trie by its name.
 * @param trie  The trie to search in.
 * @param name  The name to look for.
 *
 * @return A pointer to the element when found, or NULL otherwise.
 */
void * trie_find (struct trie_node_t *trie, const char *name);

/**
 * Returns the number of elements in the trie.
 * @param trie  The trie to analyze.
 *
 * @return The number of elements found in the trie.
 */
uint32_t trie_elements_in_trie (struct trie_node_t *trie);

/**
 * Destructor for a trie.
 * @param trie  The trie to destroy.
 */
void trie_destroy (struct trie_node_t *trie);

/**
 * Destructor for a trie.  The function passed as callback will be called upon
 * the trie node's element if it has any.
 * @param trie      The trie to destroy.
 * @param callback  The function to call on the trie node's element
 */
void trie_destroy_full (struct trie_node_t *trie, void (*callback) (void *));


#endif
