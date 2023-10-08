#pragma once

#include <memory>


class VirtualResource {
public:
    virtual void create() = 0;

    virtual void destroy() = 0;

    virtual ~VirtualResource() = default;

protected:
};

template<class RESOURCE>
class Resource : public VirtualResource {
public:

    void create() override {
        resource.create(name, desc);
    }

    ~Resource() {
    }


    void destroy() override {
    }

    Resource(const char *name, typename RESOURCE::Descriptor desc) : desc(desc), name(name) {
    }

    Resource(const char *name) : name(name) {}

    const RESOURCE &getHandle() const {
        return resource;
    }

protected:
    //std::unique_ptr<RESOURCE> resource{nullptr};
    RESOURCE resource;
    typename RESOURCE::Descriptor desc;
    const char *const name;
};

template<class RESOURCE>
class ImportedResource : public Resource<RESOURCE> {
public:
    ImportedResource(const char *name, const RESOURCE &resource) : Resource<RESOURCE>(name) {
        this->resource = resource;
    }

    void create() override {
        // do nothing
    }


    ~ImportedResource() {
        // do nothing
    }
};





