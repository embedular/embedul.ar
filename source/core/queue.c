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

#include "embedul.ar/source/core/queue.h"
#include "embedul.ar/source/core/device/board.h"


/**
 * Initializes a :c:struct:`QUEUE` instance.
 */
void QUEUE_Init (struct QUEUE *const Q)
{
    BOARD_AssertParams (Q);

    OBJECT_Clear (Q);
}


static void nodePushBack (struct QUEUE_Node *const RefNode,
                          struct QUEUE_Node *const NewNode)
{
    if (RefNode->prev)
    {
        RefNode->prev->next = NewNode;
        NewNode->prev       = RefNode->prev;
    }
    else
    {
        NewNode->prev = NULL;
    }

    RefNode->prev = NewNode;
    NewNode->next = RefNode;
}


static void nodePushFront (struct QUEUE_Node *const RefNode,
                           struct QUEUE_Node *const NewNode)
{
    if (RefNode->next)
    {
        RefNode->next->prev = NewNode;
        NewNode->next       = RefNode->next;
    }
    else
    {
        NewNode->next = NULL;
    }

    RefNode->next = NewNode;
    NewNode->prev = RefNode;
}


/**
 * Inserts a node at the Queue's back as in the following figure:
 *
 * .. image:: images/queue_push_back.drawio.svg
 *
 * :param Node: Node to insert.
 */
void QUEUE_NodePushBack (struct QUEUE *const Q, struct QUEUE_Node *const Node)
{
    BOARD_AssertParams (Q && Node);

    // Empty queue
    if (!Q->front)
    {
        BOARD_AssertState (!Q->back);
        Q->front = Node;
    }
    else
    {
        BOARD_AssertState (Q->back);
        nodePushBack (Q->back, Node);
    }

    Q->back = Node;
    ++ Q->elements;
}


/**
 * Inserts a node at the Queue's front as in the following figure:
 *
 * .. image:: images/queue_push_front.drawio.svg
 *
 * :param Node: Node to insert.
 */
void QUEUE_NodePushFront (struct QUEUE *const Q, struct QUEUE_Node *const Node)
{
    BOARD_AssertParams (Q && Node);

    // Empty queue
    if (!Q->back)
    {
        BOARD_AssertState (!Q->front);
        Q->back = Node;
    }
    else
    {
        BOARD_AssertState (Q->front);
        nodePushFront (Q->front, Node);
    }

    Q->front = Node;
    ++ Q->elements;
}


/**
 * Removes a node from the Queue's front as in the following figure:
 *
 * .. image:: images/queue_pop_front.drawio.svg
 *
 * :return: Detached node.
 */
struct QUEUE_Node * QUEUE_NodePopFront (struct QUEUE *const Q)
{
    struct QUEUE_Node * node = Q->front;
    QUEUE_NodeDetach (Q, Q->front);

    return node;
}


/**
 * Removes a node from the Queue's back as in the following figure:
 *
 * .. image:: images/queue_pop_back.drawio.svg
 *
 * :return: Detached node.
 */
struct QUEUE_Node * QUEUE_NodePopBack (struct QUEUE *const Q)
{
    struct QUEUE_Node * node = Q->back;
    QUEUE_NodeDetach (Q, Q->back);

    return node;
}


/**
 * Inserts a node at the Queue's back. An alias of :c:func:`QUEUE_NodePushBack`
 * for consistency when using the Queue as single-ended.
 *
 * :param Node: Node to insert.
 */
void QUEUE_NodeEnqueue (struct QUEUE *const Q, struct QUEUE_Node *const Node)
{
    QUEUE_NodePushBack (Q, Node);
}


/**
 * Removes a node from the Queue's front. An alias of
 * :c:func:`QUEUE_NodePopFront` for consistency when using the Queue as
 * single-ended.
 *
 * :return: Detached node.
 */
struct QUEUE_Node * QUEUE_NodeDequeue (struct QUEUE *const Q)
{
    return QUEUE_NodePopFront (Q);
}


static void trvReset (struct QUEUE_TRV *const T)
{
    T->current = (T->dir == QUEUE_TRV_Dir_FrontToBack)?
                    T->queue->front : T->queue->back;
}


/**
 * Initializes a Queue Traversal to step through all elements in the 
 * specified direction.
 *
 * :param Queue: Queue to traverse.
 * :param Dir: :c:enum:`QUEUE_TRV_Dir`, traverse direction.
 */
void QUEUE_TRV_Init (struct QUEUE_TRV *const T, struct QUEUE *const Queue,
                     const enum QUEUE_TRV_Dir Dir)
{
    BOARD_AssertParams (T && Queue);

    OBJECT_Clear (T);

    T->queue    = Queue;
    T->dir      = Dir;

    trvReset (T);
}


/**
 * Steps through a Queue returning the current position node or
 * :c:macro:`NULL` if the traversal reached the Queue back or
 * front ends, depending on traverse direction. The user
 * can detach the returned node (see :c:func:`QUEUE_NodeDetach`)
 * without affecting the ongoing node traversal, as seen in the
 * following figure:
 *
 * .. image:: images/queue_trv_detach.drawio.svg
 *
 * :return: Node at current traverse position.
 */
struct QUEUE_Node * QUEUE_TRV_Step (struct QUEUE_TRV *const T)
{
    BOARD_AssertParams (T);

    if (!T->current)
    {
        return NULL;
    }

    struct QUEUE_Node *const LastCurrent = T->current;

    // Next current. The caller may detach the returned
    // node (last current) without affecting further node traversals.
    T->current = (T->dir == QUEUE_TRV_Dir_FrontToBack)?
                    T->current->prev : T->current->next;

    return LastCurrent;
}


/**
 * Resets a Queue Traversal to the first step in the direction specified
 * at initialization.
 */
void QUEUE_TRV_Reset (struct QUEUE_TRV *const T)
{
    BOARD_AssertParams (T);
    trvReset (T);
}


static void nodeDetach (struct QUEUE_Node *const Node)
{
    // Detaching node from the chain
    if (Node->prev)
    {
        Node->prev->next = Node->next;
    }

    if (Node->next)
    {
        Node->next->prev = Node->prev;
    }

    Node->prev = NULL;
    Node->next = NULL;
}


/**
 * Detaches a node from its Queue and neighbor nodes.
 *
 * :param Node: Node to detach, usually a node returned by
 *              :c:func:`QUEUE_TRV_Step` when traversing a Queue and
 *              performing a search and extraction or node rearrangement.
 */
void QUEUE_NodeDetach (struct QUEUE *const Q, struct QUEUE_Node *const Node)
{
    BOARD_AssertParams (Q && Node);
    BOARD_AssertState  (Q->elements && Q->front && Q->back);

    const bool NodeIsHead = (Q->front == Node);
    const bool NodeIsTail = (Q->back == Node);

    // Removing the only node in the queue.
    if (NodeIsHead && NodeIsTail)
    {
        BOARD_AssertState (!Node->prev && !Node->next);
        Q->front    = NULL;
        Q->back     = NULL;
    }
    // Removing the front node. There is a node behind.
    else if (NodeIsHead)
    {
        BOARD_AssertState (Node->prev);
        Q->front = Node->prev;
    }
    // Removing the back node. There is a node ahead.
    else if (NodeIsTail)
    {
        BOARD_AssertState (Node->next);
        Q->back = Node->next;
    }

    nodeDetach (Node);
    -- Q->elements;
}
