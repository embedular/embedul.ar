/*
  embedul.ar™ embedded systems framework - http://embedul.ar
  
  [CORE] double-ended queue data structure.

  Copyright 2018-2022 Santiago Germino
  <sgermino@embedul.ar> https://www.linkedin.com/in/royconejo

  This software is provided 'as-is', without any express or implied
  warranty.  In no event will the authors be held liable for any damages
  arising from the use of this software.

  Permission is granted to anyone to use this software for any purpose,
  including commercial applications, and to alter it and redistribute it
  freely, subject to the following restrictions:

  1. The origin of this software must not be misrepresented; you must not
     claim that you wrote the original software. If you use this software
     in a product, an acknowledgment in the product documentation would be
     appreciated but is not required.
  2. Altered source versions must be plainly marked as such, and must not be
     misrepresented as being the original software.
  3. This notice may not be removed or altered from any source distribution.
*/

#pragma once

#include <stdint.h>


/**
 * Description
 * ===========
 *
 * Double-ended Queue implemented as a doubly-linked list data structure.
 * It inserts elements either to the back of the list (push back) or to the
 * front (push front) and removes them either from the front (pop front) or
 * from the back (pop back). It can also serve as a simple, single-ended
 * Queue to implement a FIFO (First-In, First Out) structure, using only two
 * operations: insert elements to the back (Enqueue) and remove them from the
 * front (Dequeue). Please see the `Wikipedia article on Queues`_ for detailed
 * information about this data structure.
 *
 * There is a **Queue Traversator™** to traverse elements either from
 * front to back or otherwise from back to front.
 *
 * A typical use case is a FIFO (First-In, First-Out) element list. For
 * example, tasks on the RetrOS scheduler waiting for their turn to execute.
 *
 * To make a structure Queueable™, the application programmer need to embed a 
 * QUEUE_Node as the first structure member as follows:
 *
 * .. literalinclude:: code-samples/queue-cast.c
 *    :language: c
 *
 * .. _`Wikipedia article on Queues`:
 *    https://en.wikipedia.org/wiki/Queue_(abstract_data_type)
 *
 *
 * API guide
 * =========
 *
 * Queue
 * -----
 *
 * Initializes an empty queue.
 *
 * | :c:func:`QUEUE_Init`
 *
 * Inserts and removes elements as a double-ended queue.
 *
 * | :c:func:`QUEUE_NodePushBack`
 * | :c:func:`QUEUE_NodePushFront`
 * | :c:func:`QUEUE_NodePopFront`
 * | :c:func:`QUEUE_NodePopBack`
 *
 * Inserts and removes elements as a single-ended queue.
 *
 * | :c:func:`QUEUE_NodeEnqueue`
 * | :c:func:`QUEUE_NodeDequeue`
 *
 * Detaches a particular node by its :c:struct:`QUEUE_Node`. Usually used
 * when traversing a queue.
 *
 * | :c:func:`QUEUE_NodeDetach`
 *
 * Traversator™
 * ------------
 *
 * Initializes by passing the queue to traverse and the desired step direction
 * to follow.
 *
 * | :c:func:`QUEUE_TRV_Init`
 *
 * Performs a step and returns the current node's :c:struct:`QUEUE_Node`.
 *
 * | :c:func:`QUEUE_TRV_Step`
 *
 * Resets traversal to the initial position.
 *
 * | :c:func:`QUEUE_TRV_Reset`
 *
 *
 * Design and development status
 * =============================
 *
 * Feature-complete.
 *
 *
 * Changelog
 * =========
 *
 * ======= ========== =================== ======================================
 * Version Date*      Author              Comment
 * ======= ========== =================== ======================================
 * 1.0.0   2022.9.7   sgermino            Initial release.
 * ======= ========== =================== ======================================
 *
 * \* Date format is Year.Month.Day.
 *
 *
 * API reference
 * =============
 */


/**
 * The user should treat this as an opaque structure. No member should be
 * directly accessed or modified.
 */
struct QUEUE_Node
{
    struct QUEUE_Node   * prev;     // Node closer to the back
    struct QUEUE_Node   * next;     // Node closer to the front
};


/**
 * The user should treat this as an opaque structure. No member should be
 * directly accessed or modified.
 */
struct QUEUE
{
    struct QUEUE_Node   * front;
    struct QUEUE_Node   * back;
    uint32_t            elements;
};


/**
 * Traverse direction.
 */
enum QUEUE_TRV_Dir
{
    /** Front to back. */
    QUEUE_TRV_Dir_FrontToBack = 0,
    /** Back to front. */
    QUEUE_TRV_Dir_BackToFront
};


/**
 * The user should treat this as an opaque structure. No member should be
 * directly accessed or modified.
 */
struct QUEUE_TRV
{
    struct QUEUE        * queue;
    struct QUEUE_Node   * current;
    enum QUEUE_TRV_Dir  dir;
};


void                QUEUE_Init              (struct QUEUE *const Q);
void                QUEUE_NodePushBack      (struct QUEUE *const Q,
                                             struct QUEUE_Node *const Node);
void                QUEUE_NodePushFront     (struct QUEUE *const Q,
                                             struct QUEUE_Node *const Node);
struct QUEUE_Node * QUEUE_NodePopFront      (struct QUEUE *const Q);
struct QUEUE_Node * QUEUE_NodePopBack       (struct QUEUE *const Q);
void                QUEUE_NodeEnqueue       (struct QUEUE *const Q,
                                             struct QUEUE_Node *const Node);
struct QUEUE_Node * QUEUE_NodeDequeue       (struct QUEUE *const Q);
void                QUEUE_TRV_Init          (struct QUEUE_TRV *const T,
                                             struct QUEUE *const Queue,
                                             const enum QUEUE_TRV_Dir Dir);
struct QUEUE_Node * QUEUE_TRV_Step          (struct QUEUE_TRV *const T);
void                QUEUE_TRV_Reset         (struct QUEUE_TRV *const T);
void                QUEUE_NodeDetach        (struct QUEUE *const Q,
                                             struct QUEUE_Node *const Node);
