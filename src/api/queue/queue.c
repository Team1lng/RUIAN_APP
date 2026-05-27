#include "queue.h"
#include "stdio.h"

// 断言宏：当条件不满足时打印错误信息，包含函数名和行号
#define XM_ASSERT(x) printf("%s : %d queue fail\n",__func__,__LINE__)

/**
 * 初始化队列头节点
 * @param queue 指向队列头节点的指针
 * 
 * 功能：将队列头节点的前驱和后继指针都指向自身，形成空循环链表
 */
void queue_initialize (queue_s *queue)
{
	queue->prev = queue;
	queue->next = queue;
}

/**
 * 在指定节点前插入新节点
 * @param entry 要插入的新节点
 * @param queue 目标位置节点（新节点将插入到该节点之前）
 * 
 * 功能：将entry节点插入到queue节点的前面，即entry成为queue的前驱节点
 */
void queue_insert (queue_s *entry, queue_s *queue)
{
	entry->prev = queue->prev;
	entry->next = queue;
	queue->prev->next = entry;
	queue->prev = entry;
}

/**
 * 从队列中删除指定节点
 * @param entry 要删除的节点
 * 
 * 功能：将entry节点从队列中移除，并将其前驱和后继指针指向自身，形成自循环
 * 保护：删除前会检查节点是否已处于自循环状态（即不在队列中）
 */
void queue_delete (queue_s *entry)
{
	if(entry->prev == entry || entry->next == entry)
	{
		XM_ASSERT (0);  // 错误：尝试删除已脱离队列的节点
	}
	if (entry->next != entry)
	{
		entry->prev->next = entry->next;
		entry->next->prev = entry->prev;
		
		entry->prev = entry;
		entry->next = entry;
	}
}

/**
 * 获取队列的第一个元素（头节点的下一个节点）
 * @param queue 队列头节点
 * @return 队列的第一个元素指针
 * 
 * 注意：队列为空时会触发断言
 */
queue_s * queue_head (queue_s *queue)
{
	if(queue->next == queue)
	{
		XM_ASSERT (0);  // 错误：尝试获取空队列的头节点
	}
	return (queue_s *)queue->next;
}

/**
 * 获取队列的最后一个元素（头节点的前一个节点）
 * @param queue 队列头节点
 * @return 队列的最后一个元素指针
 * 
 * 注意：队列为空时会触发断言
 */
queue_s * queue_tail (queue_s *queue)
{
	if(queue->next == queue)
	{
		XM_ASSERT (0);  // 错误：尝试获取空队列的尾节点
	}
	return (queue_s *)queue->prev;
}

/**
 * 删除并返回指定节点的下一个节点
 * @param queue 指定节点
 * @return 被删除节点的指针
 * 
 * 功能：将queue节点的下一个节点从队列中移除并返回
 * 注意：操作空队列或自循环节点会触发断言
 */
queue_s * queue_delete_next (queue_s *queue)
{
	volatile queue_s	*entry;
	if(queue->next == queue || queue->prev == queue)
	{
		XM_ASSERT (0);  // 错误：尝试操作空队列或无效节点
	}
	entry = queue->next;
	queue->next = entry->next;
	entry->next->prev = queue;
	
	entry->prev = (queue_s *)entry;
	entry->next = (queue_s *)entry;
	
	return((queue_s *)entry);
}

/**
 * 获取指定节点的下一个节点
 * @param queue 指定节点
 * @return 下一个节点的指针
 */
queue_s * queue_next (queue_s *queue)
{
	return (queue_s *)queue->next;
}

/**
 * 获取指定节点的前一个节点
 * @param queue 指定节点
 * @return 前一个节点的指针
 */
queue_s * queue_prev (queue_s *queue)
{
	return (queue_s *)queue->prev;
}

/**
 * 检查队列是否为空
 * @param queue 队列头节点
 * @return 1-队列为空；0-队列非空；-1-队列结构异常
 * 
 * 验证：检查队列的首尾指针是否一致，并确保结构一致性
 */
int queue_empty (queue_s *queue)
{
	if (queue->next == queue) 
	{
		if(queue->prev != queue)
		{
			XM_ASSERT(0);  // 错误：队列结构不一致（首尾指针不匹配）
			return (-1);
		}
		return(1);  // 队列为空
	}
	return(0);  // 队列非空
}