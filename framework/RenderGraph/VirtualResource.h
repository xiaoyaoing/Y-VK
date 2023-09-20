#pragma once
#include <memory>

class VirtualResource
{
public:
    void create();
};


template<class  RESOURCE>
class Resource : public  VirtualResource
{

    void create();

    
public:
    std::unique_ptr<RESOURCE> resource{nullptr};
};
