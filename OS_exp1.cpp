#define _CRT_SECURE_NO_WARNINGS
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// 定义 PCB 结构体
typedef struct PCB {
    char PID[10]; // 进程ID
    char status[10]; // 进程状态
    struct PCB* child[10]; // 子进程指针数组
    struct PCB* parent; // 父进程指针
    int priority;  // 进程优先级，0-init，1-user，2-system
    int resource_occupied[4] = { 0,0,0,0 };  // 当前占有的四类资源
    int blocked_resource_type; // 引起阻塞资源的种类
    int blocked_num[4] = { 0, 0, 0, 0 }; // 导致阻塞时申请的资源量
} PCB;

// 定义 RCB 结构体
typedef struct RCB {
    int RID; // 资源ID
    int Init_num; // 初始资源数量
    int Ava_num; // 可用资源数量
    PCB* Waiting_List[10]; // 等待队列
} RCB;

// 全局变量声明
RCB Resource_List[4]; // 资源队列
PCB* Ready_List[10]; // 就绪队列
PCB* Blocked_List[10]; // 阻塞队列
PCB* Process_List[10]; // 进程表
PCB* Running = NULL; // 当前正在运行的进程s

// 初始化资源队列
void init_resource_list() {
    for (int i = 0; i < 4; i++) {
        Resource_List[i].RID = i + 1;
        Resource_List[i].Init_num = i + 1;
        Resource_List[i].Ava_num = i + 1;
    }
}

// 初始化进程
void init() {
    PCB* init = (PCB*)malloc(sizeof(PCB));
    strcpy(init->PID, "init");
    strcpy(init->status, "running");
    init->priority = 0;
    Running = init;
    // 更新进程表
    for (int i = 0; i < 10; i++) {
        if (Process_List[i] == NULL) {
            Process_List[i] = init;
            break;
        }
    }
}

// 按优先级排序就绪队列
void sort_ready_list() {
    int n = 10;  // 假设队列最多容纳 10 个进程
    for (int i = 0; i < n - 1; i++) {
        for (int j = 0; j < n - 1 - i; j++) {
            if (Ready_List[j] != NULL && Ready_List[j + 1] != NULL) {
                if (Ready_List[j]->priority < Ready_List[j + 1]->priority) {
                    // 交换 Ready_List[j] 和 Ready_List[j + 1]
                    PCB* temp = Ready_List[j];
                    Ready_List[j] = Ready_List[j + 1];
                    Ready_List[j + 1] = temp;
                }
            }
        }
    }
}

// 按优先级排序阻塞队列
void sort_blocked_list() {
    int n = 10;  // 假设队列最多容纳 10 个进程
    for (int i = 0; i < n - 1; i++) {
        for (int j = 0; j < n - 1 - i; j++) {
            if (Blocked_List[j] != NULL && Blocked_List[j + 1] != NULL) {
                if (Blocked_List[j]->priority < Blocked_List[j + 1]->priority) {
                    // 交换 Blocked_List[j] 和 Blocked_List[j + 1]
                    PCB* temp = Blocked_List[j];
                    Blocked_List[j] = Blocked_List[j + 1];
                    Blocked_List[j + 1] = temp;
                }
            }
        }
    }
}

// 抢占进程
void Preempt(PCB* hpri_pcb, PCB* running_pcb) {
    PCB* temp = running_pcb;
    strcpy(temp->status, "ready");
    strcpy(hpri_pcb->status, "running");
    Running = hpri_pcb;
    // 更新进程表
    for (int i = 0; i < 10; i++) {
        if (Process_List[i]->PID == hpri_pcb->PID) {
            Process_List[i] = hpri_pcb;
        }
        if (Process_List[i]->PID == temp->PID) {
            Process_List[i] = temp;
        }
    }
}


// 调度策略
void Scheduler() {
    PCB* hpri_pcb = (PCB*)malloc(sizeof(PCB)); // 就绪队列中最高优先级的进程
    if (Ready_List[0] == NULL) {
        free(hpri_pcb);
        return;
    }
    else
    {
        hpri_pcb = Ready_List[0];
    }
    // 遍历Ready队列，找到最高优先级的进程
    for (int i = 1; Ready_List[i] != NULL; i++) {
        if (Ready_List[i]->priority > hpri_pcb->priority) {
            hpri_pcb = Ready_List[i];
        }
    }
    // 抢占进程
    if (Running->priority < hpri_pcb->priority || Running->status != "running" || Running == NULL) {
        Preempt(hpri_pcb, Running);
    }
}

// 撤销进程及其子进程
void Kill_Tree(PCB* p) {
    if (p == NULL) {
        return;
    }
    // 释放进程占用的资源
    for (int i = 0; i < 4; i++) {
        Resource_List[i].Ava_num += p->resource_occupied[i];  // 释放进程占用的资源
        p->resource_occupied[i] = 0;
    }
    // 从进程列表中删除进程并更新指针
    for (int i = 0; i < 10; i++) {
        if (Process_List[i] == p) {
            Process_List[i] = NULL;
            break;
        }
    }
    // 从就绪队列和阻塞队列中移除进程
    for (int i = 0; i < 10; i++) {
        if (Ready_List[i] == p) {
            for (int j = i; j < 9; j++) {
                Ready_List[j] = Ready_List[j + 1];
            }
            Ready_List[9] = NULL; // 清空最后一位
            break;
        }
    }
    for (int i = 0; i < 10; i++) {
        if (Blocked_List[i] == p) {
            for (int j = i; j < 9; j++) {
                Blocked_List[j] = Blocked_List[j + 1];
            }
            Blocked_List[9] = NULL; // 清空最后一位
            break;
        }
    }
    // 释放进程的内存
    free(p);
}

// 创建进程
void Create(char* PID, PCB* parent, PCB* child, int priority) {
    PCB* new_pcb = (PCB*)malloc(sizeof(PCB));
    strcpy(new_pcb->PID, PID);
    for (int i = 0; i < 4; i++) {
        new_pcb->resource_occupied[i] = 0;
        new_pcb->blocked_num[i] = 0;
    }

    if (parent != NULL) {
        new_pcb->parent = parent;
    }
    else {
        new_pcb->parent = NULL;
    }

    if (child != NULL) {
        new_pcb->child[0] = child;
    }

    new_pcb->priority = priority;

    int found_empty_slot = 0;
    for (int i = 0; i < 10; i++) {  // 假设队列最多容纳 10 个进程
        if (Ready_List[i]->PID == PID || Running->PID == PID) {
            printf("已存在相同名称进程，无法创建！\n");
            free(new_pcb);
            return;
        }
        if (Ready_List[i] == NULL) {
            Ready_List[i] = new_pcb;
            // 如果没有进程在运行，立即将该进程设置为运行状态
            if (Running == NULL) {
                Running = new_pcb;
                strcpy(new_pcb->status, "running");
            }
            else {
                strcpy(new_pcb->status, "ready");  // 否则设置为 ready
            }

            found_empty_slot = 1;
            break;
        }
    }

    // 如果就绪队列中无空位
    if (!found_empty_slot) {
        printf("就绪队列已满，无法创建更多进程！\n");
        free(new_pcb);
        return;
    }

    printf("进程 %s 已创建，优先级为 %d\n", new_pcb->PID, new_pcb->priority);

    // 将新建进程添加至进程表中
    for (int i = 0; i < 10; i++) {
        if (Process_List[i] == NULL) {
            Process_List[i] = new_pcb;
            break;
        }
    }
    Scheduler();
}

// 释放资源
PCB* Release(char* PID) {
    PCB* temp = NULL;
    for (int i = 0; i < 10; i++) {
        if (Process_List[i] != NULL && strcmp(Process_List[i]->PID, PID) == 0) {
            temp = Process_List[i];
            for (int j = 0; j < 4; j++) {
                Resource_List[j].Ava_num += temp->resource_occupied[j];
                temp->resource_occupied[j] = 0;
            }
            break;
        }
    }

    // 检查阻塞队列，唤醒进程
    for (int i = 0; i < 10; i++) {
        if (Blocked_List[i] != NULL && Blocked_List[i]->blocked_resource_type != 0) {
            int block_resource_type = Blocked_List[i]->blocked_resource_type;
            if (Blocked_List[i]->blocked_num[block_resource_type - 1] <= Resource_List[block_resource_type - 1].Ava_num) {
                strcpy(Blocked_List[i]->status, "ready");
                PCB* temp = Blocked_List[i]; // 存储将被阻塞队列删除的待唤醒进程
                Blocked_List[i] = NULL; // 删除后用 NULL 填充
                for (int i = 0; i < 10; i++) {
                    if (Ready_List[i] == NULL) {
                        Ready_List[i] = temp; // 添加至就绪队列
                        break;
                    }
                }
                sort_blocked_list();
                break;
            }
        }
    }
    Scheduler();
    return temp;
}

// 销毁进程
void Destroy(char* PID) {
    PCB* p = NULL;

    // 根据 pid 找到进程控制块
    for (int i = 0; i < 10; i++) {
        if (Process_List[i] != NULL && strcmp(Process_List[i]->PID, PID) == 0) {
            p = Process_List[i];
            break;
        }
    }

    if (p == NULL) {
        printf("未找到 PID 为 %s 的进程！\n", PID);
        return;
    }

    // 判断要销毁进程的状态
    if (!strcmp(p->status, "blocked"))
    {
        for (int i = 0; i < 4; i++)
        {
            Resource_List[i].Ava_num -= p->blocked_num[i];
        }
    }
    // 释放p占用的资源
    Release(p->PID);
    // 调用 Kill_Tree 撤销进程及其子进程
    Kill_Tree(p);

    // 调度其他进程执行
    Scheduler();
}
 //请求资源
void Request(int RID, int num) {

    if (Running == NULL) {
        printf("无正在运行的进程，请求资源操作不成立\n");
        return;
    }

    if (Resource_List[RID - 1].Ava_num < num) {
        printf("无足够资源分配\n");
        Running->blocked_resource_type = RID;
        Running->resource_occupied[RID - 1] += num;
        strcpy(Running->status, "blocked");
        Running->blocked_num[RID - 1] = num;

        // 更新进程表
        for (int i = 0; i < 10; i++)
        {
            if (Process_List[i]->PID == Running->PID) {
                Process_List[i] = Running;
            }
            break;

        }

        for (int i = 0; i < 10; i++) {
            // 从就绪队列中移除Running
            if (Ready_List[i]->PID == Running->PID) {
                // 将就绪队列中后续的元素向前移动一位
                for (int j = i; j < 9; j++) {
                    Ready_List[j] = Ready_List[j + 1];
                }
                Ready_List[9] = NULL; // 队列最后一位清空
            }

            if (Blocked_List[i] == NULL) {
                Blocked_List[i] = Running; // 添加到阻塞队列
                Running = Ready_List[0];
                break;
            }
        }
        Scheduler();
    }
    else {
        Running->resource_occupied[RID - 1] += num;
        Resource_List[RID - 1].Ava_num -= num;
    }
    sort_blocked_list();
}

// 超时调度
void time_out() {
    // 找到当前正在运行的进程 q
    PCB* q = Running;
    if (q == NULL) {
        printf("当前没有正在运行的进程！\n");
        return;
    }
    // 在就绪队列中找到并移除 q
    int found = 0;
    for (int i = 0; i < 10; i++) {
        if (Ready_List[i] == q) {
            // 从就绪队列中移除 q
            found = 1;
            // 将就绪队列中后续的元素向前移动一位
            for (int j = i; j < 9; j++) {
                Ready_List[j] = Ready_List[j + 1];
            }
            Ready_List[9] = NULL; // 队列最后一位清空
            break;
        }
    }
    if (!found) {
        printf("无法在就绪队列中找到当前运行的进程！\n");
        return;
    }
    // 将 q 的状态改为 "ready"
    strcpy(q->status, "ready");
    // 将 q 重新插入到就绪队列的尾部
    for (int i = 0; i < 10; i++) {
        if (Ready_List[i] == NULL) { // 找到就绪队列中的空位
            Ready_List[i] = q;
            break;
        }
    }
    // 调用调度器进行进程调度
    Scheduler();
}

// 列出所有进程信息
void list_all_process_and_status() {
    for (int i = 0; i < 10; i++) {
        if (Process_List[i] != NULL) {
            printf("PID: %s, Status: %s, Priority: %d\n", Process_List[i]->PID, Process_List[i]->status, Process_List[i]->priority);
        }
    }
}

// 列出所有资源信息
void list_all_resource_and_status() {
    for (int i = 0; i < 4; i++) {
        printf("RID: %d, Available: %d\n", Resource_List[i].RID, Resource_List[i].Ava_num);
    }
}

// 检查进程信息
void check_process_info(char* PID) {
    for (int i = 0; i < 10; i++) {
        if (Process_List[i] != NULL && strcmp(Process_List[i]->PID, PID) == 0) {
            printf("PID: %s, Status: %s, Priority: %d\n", Process_List[i]->PID, Process_List[i]->status, Process_List[i]->priority);
            break;
        }
    }
}

// 主函数
int main() {
    init_resource_list();
    init();
    if (Running != NULL) {
        printf("%s is running\n", Running->PID);
    }
    while (1) {
        char command[20];
        printf("shell> ");
        fgets(command, 20, stdin);
        sort_ready_list();

        if (strncmp(command, "cr", 2) == 0) {
            char PID[10] = {" "};
            int priority;
            sscanf(command, "cr %s %d", PID, &priority);
            Create(PID, Running, NULL, priority);
        }
        else if (strncmp(command, "de", 2) == 0) {
            char PID[10] = {" "};
            sscanf(command, "de %s", PID);
            Destroy(PID);
        }
        else if (strncmp(command, "to", 2) == 0) {
            time_out();
        }
        else if (strncmp(command, "req", 3) == 0) {
            int RID, num;
            sscanf(command, "req R%d %d", &RID, &num);
            Request(RID, num);
        }
        else if (strncmp(command, "rel", 3) == 0) {
            Release(Running->PID);
        }
        else if (strncmp(command, "lsp", 3) == 0) {
            list_all_process_and_status();
        }
        else if (strncmp(command, "lsr", 3) == 0) {
            list_all_resource_and_status();
        }
        else if (strncmp(command, "cp", 2) == 0) {
            char PID[10] = {" "};
            sscanf(command, "cp %s", PID);
            check_process_info(PID);
        }
        if (Running != NULL) {
            printf("%s is running\n", Running->PID);
        }
    }
    return 0;
}
