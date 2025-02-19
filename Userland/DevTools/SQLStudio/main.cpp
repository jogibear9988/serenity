/*
 * Copyright (c) 2022, Dylan Katz <dykatz@uw.edu>
 *
 * SPDX-License-Identifier: BSD-2-Clause
 */

#include <AK/StringUtils.h>
#include <LibCore/ArgsParser.h>
#include <LibGUI/Application.h>
#include <LibGUI/Icon.h>
#include <LibGUI/Window.h>
#include <LibMain/Main.h>

#include "MainWidget.h"

ErrorOr<int> serenity_main(Main::Arguments arguments)
{
    StringView file_to_open;

    Core::ArgsParser args_parser;
    args_parser.add_positional_argument(file_to_open, "Path to SQL script or DB", "file", Core::ArgsParser::Required::No);
    args_parser.parse(arguments);

    auto app = TRY(GUI::Application::try_create(arguments));

    auto app_icon = GUI::Icon::default_icon("app-sql-studio"sv);

    auto window = TRY(GUI::Window::try_create());
    window->resize(640, 480);
    window->set_icon(app_icon.bitmap_for_size(16));
    window->set_title("SQL Studio");

    auto main_widget = TRY(window->set_main_widget<MainWidget>());
    main_widget->initialize_menu(window);

    window->on_close_request = [&] {
        if (main_widget->request_close())
            return GUI::Window::CloseRequestDecision::Close;
        return GUI::Window::CloseRequestDecision::StayOpen;
    };

    if (!file_to_open.is_empty()) {
        auto path = LexicalPath(file_to_open);
        main_widget->open_script_from_file(path);
    } else {
        main_widget->open_new_script();
    }

    window->show();
    return app->exec();
}
