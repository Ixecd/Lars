#!/bin/bash
# 由contacts.proto 生成 contacts.pb.cc 和 contacts.pb.h
protoc --cpp_out=. ./*.proto