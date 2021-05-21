#ifndef FireworksWeb_Core_CmsShowMainBase_h
#define FireworksWeb_Core_CmsShowMainBase_h

class CmsShowMainBase
{
public:
    CmsShowMainBase();
    virtual ~CmsShowMainBase() noexcept(false){};

    bool getLoop() const { return m_loop; }
    void setPlayLoop(bool);


protected:
    bool  m_loop {true };
};
#endif
