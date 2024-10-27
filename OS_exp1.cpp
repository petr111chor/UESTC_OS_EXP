#define _CRT_SECURE_NO_WARNINGS
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// ���� PCB �ṹ��
typedef struct PCB {
    char PID[10]; // ����ID
    char status[10]; // ����״̬
    struct PCB* child[10]; // �ӽ���ָ������
    struct PCB* parent; // ������ָ��
    int priority;  // �������ȼ���0-init��1-user��2-system
    int resource_occupied[4] = { 0,0,0,0 };  // ��ǰռ�е�������Դ
    int blocked_resource_type; // ����������Դ������
    int blocked_num[4] = { 0, 0, 0, 0 }; // ��������ʱ�������Դ��
} PCB;

// ���� RCB �ṹ��
typedef struct RCB {
    int RID; // ��ԴID
    int Init_num; // ��ʼ��Դ����
    int Ava_num; // ������Դ����
    PCB* Waiting_List[10]; // �ȴ�����
} RCB;

// ȫ�ֱ�������
RCB Resource_List[4]; // ��Դ����
PCB* Ready_List[10]; // ��������
PCB* Blocked_List[10]; // ��������
PCB* Process_List[10]; // ���̱�
PCB* Running = NULL; // ��ǰ�������еĽ���s

// ��ʼ����Դ����
void init_resource_list() {
    for (int i = 0; i < 4; i++) {
        Resource_List[i].RID = i + 1;
        Resource_List[i].Init_num = i + 1;
        Resource_List[i].Ava_num = i + 1;
    }
}

// ��ʼ������
void init() {
    PCB* init = (PCB*)malloc(sizeof(PCB));
    strcpy(init->PID, "init");
    strcpy(init->status, "running");
    init->priority = 0;
    Running = init;
    // ���½��̱�
    for (int i = 0; i < 10; i++) {
        if (Process_List[i] == NULL) {
            Process_List[i] = init;
            break;
        }
    }
}

// �����ȼ������������
void sort_ready_list() {
    int n = 10;  // �������������� 10 ������
    for (int i = 0; i < n - 1; i++) {
        for (int j = 0; j < n - 1 - i; j++) {
            if (Ready_List[j] != NULL && Ready_List[j + 1] != NULL) {
                if (Ready_List[j]->priority < Ready_List[j + 1]->priority) {
                    // ���� Ready_List[j] �� Ready_List[j + 1]
                    PCB* temp = Ready_List[j];
                    Ready_List[j] = Ready_List[j + 1];
                    Ready_List[j + 1] = temp;
                }
            }
        }
    }
}

// �����ȼ�������������
void sort_blocked_list() {
    int n = 10;  // �������������� 10 ������
    for (int i = 0; i < n - 1; i++) {
        for (int j = 0; j < n - 1 - i; j++) {
            if (Blocked_List[j] != NULL && Blocked_List[j + 1] != NULL) {
                if (Blocked_List[j]->priority < Blocked_List[j + 1]->priority) {
                    // ���� Blocked_List[j] �� Blocked_List[j + 1]
                    PCB* temp = Blocked_List[j];
                    Blocked_List[j] = Blocked_List[j + 1];
                    Blocked_List[j + 1] = temp;
                }
            }
        }
    }
}

// ��ռ����
void Preempt(PCB* hpri_pcb, PCB* running_pcb) {
    PCB* temp = running_pcb;
    strcpy(temp->status, "ready");
    strcpy(hpri_pcb->status, "running");
    Running = hpri_pcb;
    // ���½��̱�
    for (int i = 0; i < 10; i++) {
        if (Process_List[i]->PID == hpri_pcb->PID) {
            Process_List[i] = hpri_pcb;
        }
        if (Process_List[i]->PID == temp->PID) {
            Process_List[i] = temp;
        }
    }
}


// ���Ȳ���
void Scheduler() {
    PCB* hpri_pcb = (PCB*)malloc(sizeof(PCB)); // ����������������ȼ��Ľ���
    if (Ready_List[0] == NULL) {
        free(hpri_pcb);
        return;
    }
    else
    {
        hpri_pcb = Ready_List[0];
    }
    // ����Ready���У��ҵ�������ȼ��Ľ���
    for (int i = 1; Ready_List[i] != NULL; i++) {
        if (Ready_List[i]->priority > hpri_pcb->priority) {
            hpri_pcb = Ready_List[i];
        }
    }
    // ��ռ����
    if (Running->priority < hpri_pcb->priority || Running->status != "running" || Running == NULL) {
        Preempt(hpri_pcb, Running);
    }
}

// �������̼����ӽ���
void Kill_Tree(PCB* p) {
    if (p == NULL) {
        return;
    }
    // �ͷŽ���ռ�õ���Դ
    for (int i = 0; i < 4; i++) {
        Resource_List[i].Ava_num += p->resource_occupied[i];  // �ͷŽ���ռ�õ���Դ
        p->resource_occupied[i] = 0;
    }
    // �ӽ����б���ɾ�����̲�����ָ��
    for (int i = 0; i < 10; i++) {
        if (Process_List[i] == p) {
            Process_List[i] = NULL;
            break;
        }
    }
    // �Ӿ������к������������Ƴ�����
    for (int i = 0; i < 10; i++) {
        if (Ready_List[i] == p) {
            for (int j = i; j < 9; j++) {
                Ready_List[j] = Ready_List[j + 1];
            }
            Ready_List[9] = NULL; // ������һλ
            break;
        }
    }
    for (int i = 0; i < 10; i++) {
        if (Blocked_List[i] == p) {
            for (int j = i; j < 9; j++) {
                Blocked_List[j] = Blocked_List[j + 1];
            }
            Blocked_List[9] = NULL; // ������һλ
            break;
        }
    }
    // �ͷŽ��̵��ڴ�
    free(p);
}

// ��������
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
    for (int i = 0; i < 10; i++) {  // �������������� 10 ������
        if (Ready_List[i]->PID == PID || Running->PID == PID) {
            printf("�Ѵ�����ͬ���ƽ��̣��޷�������\n");
            free(new_pcb);
            return;
        }
        if (Ready_List[i] == NULL) {
            Ready_List[i] = new_pcb;
            // ���û�н��������У��������ý�������Ϊ����״̬
            if (Running == NULL) {
                Running = new_pcb;
                strcpy(new_pcb->status, "running");
            }
            else {
                strcpy(new_pcb->status, "ready");  // ��������Ϊ ready
            }

            found_empty_slot = 1;
            break;
        }
    }

    // ��������������޿�λ
    if (!found_empty_slot) {
        printf("���������������޷�����������̣�\n");
        free(new_pcb);
        return;
    }

    printf("���� %s �Ѵ��������ȼ�Ϊ %d\n", new_pcb->PID, new_pcb->priority);

    // ���½�������������̱���
    for (int i = 0; i < 10; i++) {
        if (Process_List[i] == NULL) {
            Process_List[i] = new_pcb;
            break;
        }
    }
    Scheduler();
}

// �ͷ���Դ
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

    // ����������У����ѽ���
    for (int i = 0; i < 10; i++) {
        if (Blocked_List[i] != NULL && Blocked_List[i]->blocked_resource_type != 0) {
            int block_resource_type = Blocked_List[i]->blocked_resource_type;
            if (Blocked_List[i]->blocked_num[block_resource_type - 1] <= Resource_List[block_resource_type - 1].Ava_num) {
                strcpy(Blocked_List[i]->status, "ready");
                PCB* temp = Blocked_List[i]; // �洢������������ɾ���Ĵ����ѽ���
                Blocked_List[i] = NULL; // ɾ������ NULL ���
                for (int i = 0; i < 10; i++) {
                    if (Ready_List[i] == NULL) {
                        Ready_List[i] = temp; // �������������
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

// ���ٽ���
void Destroy(char* PID) {
    PCB* p = NULL;

    // ���� pid �ҵ����̿��ƿ�
    for (int i = 0; i < 10; i++) {
        if (Process_List[i] != NULL && strcmp(Process_List[i]->PID, PID) == 0) {
            p = Process_List[i];
            break;
        }
    }

    if (p == NULL) {
        printf("δ�ҵ� PID Ϊ %s �Ľ��̣�\n", PID);
        return;
    }

    // �ж�Ҫ���ٽ��̵�״̬
    if (!strcmp(p->status, "blocked"))
    {
        for (int i = 0; i < 4; i++)
        {
            Resource_List[i].Ava_num -= p->blocked_num[i];
        }
    }
    // �ͷ�pռ�õ���Դ
    Release(p->PID);
    // ���� Kill_Tree �������̼����ӽ���
    Kill_Tree(p);

    // ������������ִ��
    Scheduler();
}
 //������Դ
void Request(int RID, int num) {

    if (Running == NULL) {
        printf("���������еĽ��̣�������Դ����������\n");
        return;
    }

    if (Resource_List[RID - 1].Ava_num < num) {
        printf("���㹻��Դ����\n");
        Running->blocked_resource_type = RID;
        Running->resource_occupied[RID - 1] += num;
        strcpy(Running->status, "blocked");
        Running->blocked_num[RID - 1] = num;

        // ���½��̱�
        for (int i = 0; i < 10; i++)
        {
            if (Process_List[i]->PID == Running->PID) {
                Process_List[i] = Running;
            }
            break;

        }

        for (int i = 0; i < 10; i++) {
            // �Ӿ����������Ƴ�Running
            if (Ready_List[i]->PID == Running->PID) {
                // �����������к�����Ԫ����ǰ�ƶ�һλ
                for (int j = i; j < 9; j++) {
                    Ready_List[j] = Ready_List[j + 1];
                }
                Ready_List[9] = NULL; // �������һλ���
            }

            if (Blocked_List[i] == NULL) {
                Blocked_List[i] = Running; // ��ӵ���������
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

// ��ʱ����
void time_out() {
    // �ҵ���ǰ�������еĽ��� q
    PCB* q = Running;
    if (q == NULL) {
        printf("��ǰû���������еĽ��̣�\n");
        return;
    }
    // �ھ����������ҵ����Ƴ� q
    int found = 0;
    for (int i = 0; i < 10; i++) {
        if (Ready_List[i] == q) {
            // �Ӿ����������Ƴ� q
            found = 1;
            // �����������к�����Ԫ����ǰ�ƶ�һλ
            for (int j = i; j < 9; j++) {
                Ready_List[j] = Ready_List[j + 1];
            }
            Ready_List[9] = NULL; // �������һλ���
            break;
        }
    }
    if (!found) {
        printf("�޷��ھ����������ҵ���ǰ���еĽ��̣�\n");
        return;
    }
    // �� q ��״̬��Ϊ "ready"
    strcpy(q->status, "ready");
    // �� q ���²��뵽�������е�β��
    for (int i = 0; i < 10; i++) {
        if (Ready_List[i] == NULL) { // �ҵ����������еĿ�λ
            Ready_List[i] = q;
            break;
        }
    }
    // ���õ��������н��̵���
    Scheduler();
}

// �г����н�����Ϣ
void list_all_process_and_status() {
    for (int i = 0; i < 10; i++) {
        if (Process_List[i] != NULL) {
            printf("PID: %s, Status: %s, Priority: %d\n", Process_List[i]->PID, Process_List[i]->status, Process_List[i]->priority);
        }
    }
}

// �г�������Դ��Ϣ
void list_all_resource_and_status() {
    for (int i = 0; i < 4; i++) {
        printf("RID: %d, Available: %d\n", Resource_List[i].RID, Resource_List[i].Ava_num);
    }
}

// ��������Ϣ
void check_process_info(char* PID) {
    for (int i = 0; i < 10; i++) {
        if (Process_List[i] != NULL && strcmp(Process_List[i]->PID, PID) == 0) {
            printf("PID: %s, Status: %s, Priority: %d\n", Process_List[i]->PID, Process_List[i]->status, Process_List[i]->priority);
            break;
        }
    }
}

// ������
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
