/** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
****************************************************************************/

#include <QApplication>
#include <QPushButton>
#include "dialog.h"

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);
	ChatDialog gui;
    gui.show();
    return app.exec();
} 
