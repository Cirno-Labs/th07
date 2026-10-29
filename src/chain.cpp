// The chains, from 0x42fa60.

#include "chain.h"
#include "unsorted.h"

Chain::~Chain()
{
}

ChainElem::ChainElem()
{
    this->prev = NULL;
    this->next = NULL;
    this->callback = NULL;
    this->self = this;
    this->addedCallback = NULL;
    this->deletedCallback = NULL;
    this->priority = 0;
    this->allocated = 0;
}

ChainElem::~ChainElem()
{
    if (this->deletedCallback != NULL)
    {
        this->deletedCallback(this->arg);
    }
    this->prev = NULL;
    this->next = NULL;
    this->callback = NULL;
    this->addedCallback = NULL;
    this->deletedCallback = NULL;
}

Chain::Chain()
{
}

int Chain::AddToCalcChain(ChainElem *elem, int priority)
{
    ChainElem *cur = &this->calcChain;

    elem->priority = priority;
    while (cur->next != NULL)
    {
        if (cur->priority > priority)
        {
            break;
        }
        cur = cur->next;
    }
    if (cur->priority > priority)
    {
        elem->next = cur;
        elem->prev = cur->prev;
        if (elem->prev != NULL)
        {
            elem->prev->next = elem;
        }
        cur->prev = elem;
    }
    else
    {
        elem->next = NULL;
        elem->prev = cur;
        cur->next = elem;
    }
    if (elem->addedCallback != NULL)
    {
        int result = elem->addedCallback(elem->arg);
        elem->addedCallback = NULL;
        return result;
    }
    return 0;
}

int Chain::AddToDrawChain(ChainElem *elem, int priority)
{
    ChainElem *cur = &this->drawChain;

    elem->priority = priority;
    while (cur->next != NULL)
    {
        if (cur->priority > priority)
        {
            break;
        }
        cur = cur->next;
    }
    if (cur->priority > priority)
    {
        elem->next = cur;
        elem->prev = cur->prev;
        if (elem->prev != NULL)
        {
            elem->prev->next = elem;
        }
        cur->prev = elem;
    }
    else
    {
        elem->next = NULL;
        elem->prev = cur;
        cur->next = elem;
    }
    if (elem->addedCallback != NULL)
    {
        return elem->addedCallback(elem->arg);
    }
    return 0;
}

int Chain::RunCalcChain()
{
    ChainElem *current;
    int updateCount;
    ChainElem *tmp;

restart:
    updateCount = 0;
    current = &this->calcChain;
    while (current != NULL)
    {
        if (current->callback != NULL)
        {
        again:
            switch (current->callback(current->arg))
            {
            case 0:
                tmp = current;
                current = current->next;
                this->Remove(tmp);
                updateCount++;
                continue;
            case 2:
                goto again;
            case 4:
                return 0;
            case 3:
                return 1;
            case 5:
                return -1;
            case 6:
                goto restart;
            default:
                updateCount++;
                break;
            }
        }
        current = current->next;
    }
    return updateCount;
}

int Chain::RunDrawChain()
{
    ChainElem *current;
    int updateCount;
    ChainElem *tmp;

    updateCount = 0;
    current = &this->drawChain;
    while (current != NULL)
    {
        if (current->callback != NULL)
        {
        again:
            switch (current->callback(current->arg))
            {
            case 0:
                tmp = current;
                current = current->next;
                this->Remove(tmp);
                updateCount++;
                continue;
            case 2:
                goto again;
            case 4:
                return 0;
            case 3:
                return 1;
            case 5:
                return -1;
            default:
                updateCount++;
                break;
            }
        }
        current = current->next;
    }
    return updateCount;
}

void Chain::ReleaseList(ChainElem *root)
{
    ChainElem list;
    ChainElem *tail;
    ChainElem *tmp;
    ChainElem *cur;

    tail = new ChainElem();
    list.next = tail;
    cur = root;
    while (cur != NULL)
    {
        tail->self = cur;
        tail->next = new ChainElem();
        tail = tail->next;
        cur = cur->next;
    }
    cur = &list;
    while (cur != NULL)
    {
        this->Remove(cur->self);
        cur = cur->next;
    }
    tail = list.next;
    while (tail != NULL)
    {
        tmp = tail->next;
        delete tail;
        tail = NULL;
        tail = tmp;
    }
}

void Chain::Release()
{
    this->ReleaseList(&this->calcChain);
    this->ReleaseList(&this->drawChain);
}

ChainElem *Chain::CreateElem(ChainCallback callback)
{
    ChainElem *elem = new ChainElem();
    elem->SetCallback(callback);
    elem->allocated = 1;
    return elem;
}

void Chain::Remove(ChainElem *elem)
{
    BOOL isDraw = FALSE;
    ChainElem *cur;
    ChainCallback callback;

    if (elem == NULL)
    {
        return;
    }
    cur = &this->calcChain;
    while (cur != NULL)
    {
        if (cur == elem)
        {
            goto found;
        }
        cur = cur->next;
    }
    isDraw = TRUE;
    cur = &this->drawChain;
    while (cur != NULL)
    {
        if (cur == elem)
        {
            goto found;
        }
        cur = cur->next;
    }
    return;

found:
    // isDraw is set but no longer used: th06 logged which chain the
    // element was cut from.
    if (elem->prev != NULL)
    {
        elem->callback = NULL;
        elem->prev->next = elem->next;
        if (elem->next != NULL)
        {
            elem->next->prev = elem->prev;
        }
        elem->prev = NULL;
        elem->next = NULL;
        if (elem->allocated)
        {
            delete elem;
            elem = NULL;
        }
        else if (elem->deletedCallback != NULL)
        {
            callback = elem->deletedCallback;
            elem->deletedCallback = NULL;
            callback(elem->arg);
        }
    }
}
