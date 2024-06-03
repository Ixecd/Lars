#include <iostream>

#include "contacts.pb.h"

int main() {
    std::string people_str;

    std::string student_str;
    int student_id;
    int student_opt;

    {
        // 对⼀个联系⼈的信息使⽤ PB 进⾏序列化，并将结果打印出来。
        contacts::PeopleInfo people;
        people.set_name("张珊");
        people.set_age(20);
        if (!people.SerializeToString(&people_str)) {
            std::cerr << "序列化联系⼈失败！" << std::endl;
            return -1;
        }
        std::cout << "序列化成功，结果：" << people_str << std::endl;
    }


    {
        // 对一个student信息使用PB 进行序列化
        contacts::StudentInfo student;
        student.set_id(1);
        student.set_str("qc");
        student.set_opt(3);
        if (!student.SerializeToString(&student_str)) {
            std::cerr << "序列化学生失败! " << std::endl;
            return -1;
        } 
        std::cout << "序列化成功 : " << student_str << " " <<  student.id() << " " << student.str() << " " << student.opt() << std::endl;
    }


    {
        // 对序列化后的内容使⽤ PB 进⾏反序列，解析出联系⼈信息并打印出来。
        contacts::PeopleInfo people;
        if (!people.ParseFromString(people_str)) {
            std::cerr << "反序列化联系⼈失败！" << std::endl;
            return -1;
        }
        std::cout << "反序列化成功！" << std::endl
                  << "姓名： " << people.name() << std::endl
                  << "年龄： " << people.age() << std::endl;
    }

    {
        // 反序列
        contacts::StudentInfo student;
        if (!student.ParseFromString(student_str)) {
            std::cerr << "反序列化学生失败! " << std::endl;
            return -1;
        }
        std::cout << "反序列化成功！ " << std::endl <<
        "姓名: " << student.id() << std::endl << 
        "str: " << student.str() << std::endl <<
        "opt: " << student.opt() << std::endl;
    }

    return 0;
}
