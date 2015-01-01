/* This file is subject to the terms and conditions defined in
 * file 'LICENSE.txt', which is part of this source code package.
 */
#pragma once

class Page;

class AttractMode
{
public:
    AttractMode();
    void SetIdleTime(float time);
    void Reset();
    void Update(float dt, Page &page);

private:
    bool IsActive;
    float ElapsedTime;
    float ActiveTime;
    float IdleTime;
    
};
