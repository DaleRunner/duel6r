/*
* Copyright (c) 2006, Ondrej Danek (www.ondrej-danek.net)
* All rights reserved.
* Redistribution and use in source and binary forms, with or without
* modification, are permitted provided that the following conditions are met:
*
*     * Redistributions of source code must retain the above copyright
*       notice, this list of conditions and the following disclaimer.
*     * Redistributions in binary form must reproduce the above copyright
*       notice, this list of conditions and the following disclaimer in the
*       documentation and/or other materials provided with the distribution.
*     * Neither the name of Ondrej Danek nor the
*       names of its contributors may be used to endorse or promote products
*       derived from this software without specific prior written permission.
*
* THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
* AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
* IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
* ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE
* LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
* CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE
* GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
* HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
* LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT
* OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#include <SDL2/SDL_opengl.h>
#include "Control.h"
#include "Desktop.h"

namespace Duel6
{
	namespace Gui
	{
		namespace
		{
			Color frameLightColor(235, 235, 235), frameDarkColor(0, 0, 0);
		}

		Control::Control(Desktop& desk)
		{
			desk.addControl(this);
		}

		void Control::drawFrame(Int32 x, Int32 y, Int32 w, Int32 h, bool p) const
		{
			w--;
			h--;

			glBegin(GL_LINES);
			if (p)
			{
				glColor3ub(frameDarkColor.getRed(), frameDarkColor.getGreen(), frameDarkColor.getBlue());
			}
			else
			{
				glColor3ub(frameLightColor.getRed(), frameLightColor.getGreen(), frameLightColor.getBlue());;
			}

			glVertex2i(x, y); glVertex2i(x, y - h);
			glVertex2i(x + 1, y); glVertex2i(x + 1, y - h + 1);
			glVertex2i(x, y); glVertex2i(x + w, y);
			glVertex2i(x, y - 1); glVertex2i(x + w - 1, y - 1);

			if (p)
			{
				glColor3ub(frameLightColor.getRed(), frameLightColor.getGreen(), frameLightColor.getBlue());;
			}
			else
			{
				glColor3ub(frameDarkColor.getRed(), frameDarkColor.getGreen(), frameDarkColor.getBlue());
			}


			glVertex2i(x + w, y - h); glVertex2i(x + w, y);
			glVertex2i(x + w - 1, y - h); glVertex2i(x + w - 1, y - 1);
			glVertex2i(x + w, y - h); glVertex2i(x, y - h);
			glVertex2i(x + w, y - h + 1); glVertex2i(x + 1, y - h + 1);
			glEnd();
		}
	}
}
