<div class="row">
   <div class="col-md-12" ><img width="100%" src="data/<%=page_id%>/bg.jpg"/></div>
</div>


<nav role="navigation" class="navbar navbar-default">
    <div class="navbar-header">
                <button type="button" data-target="#navbarCollapse" data-toggle="collapse" class="navbar-toggle">
                    <span class="sr-only">Toggle navigation</span>
                    <span class="icon-bar"></span>
                    <span class="icon-bar"></span>
                    <span class="icon-bar"></span>
                </button>
                <a href="page/<%=page_id %>/" class="navbar-brand"><%=nav_brand%></a>
    </div>

    <div id="navbarCollapse" class="collapse navbar-collapse">
        <ul class="nav navbar-nav">
        <% /*
        <?php foreach( $nav_menu as $menu_id => $menu_item ) { ?>
                <li>
                <a href="<?php echo $menu_item["link"] ?>"
                <?php if ( $current_page == $menu_id ) echo ' class="active"' ; ?>
                >
                <?php echo $menu_item["name"] ; ?>
                        </a>
                </li>
                <?php } ?>
         </ul>

                 <ul class="nav navbar-nav navbar-right">
                <?php if ( is_logged_in() ) { ?>
                        <li class="dropdown">
                                <a href="#" class="dropdown-toggle" data-toggle="dropdown">
                                        <span class="glyphicon glyphicon-user"></span>
                                        <strong><?php echo get_user_name() ?></strong>
                    <span class="glyphicon glyphicon-chevron-down"></span>
                        </a>
                        <ul class="dropdown-menu" role="menu">
                        <li>
                                <a href="#" id="logout">Logout</a>
                        </li>
                </ul>
                        </li>
                    <?php } else { ?>
                    <li><a href="#login-modal" data-toggle="modal">Login</a></li>
                <?php } ?>
                */
                %>
                  </ul>
        </div>
</nav>

<div class="modal fade" id="login-modal" tabindex="-1" role="dialog" aria-labelledby="myModalLabel" aria-hidden="true" style="display: none;">
        <div class="modal-dialog">
                <div class="modal-content">
                        <div class="modal-header" align="center">
                                <button type="button" class="close" data-dismiss="modal" aria-label="Close">
                                        <span class="glyphicon glyphicon-remove" aria-hidden="true"></span>
                                </button>
                        </div>

                        <form id="login-form" name="login-form" action="#" method="post">
                                <div class="modal-body">
                                        <div id="div-login-msg">
                                                <div id="icon-login-msg" class="glyphicon glyphicon-chevron-right"></div>
                                                <span id="text-login-msg">Type your username and password.</span>
                                        </div>
                                        <input id="username" name="username" class="form-control" type="text" placeholder="Username" required>
                                        <input id="password" name="password" class="form-control" type="password" placeholder="Password" required>
                                    <div class="modal-footer">
                                                <div>
                                                        <button type="submit" class="btn btn-primary btn-lg btn-block">Login</button>
                                                </div>
                                    </div>
                            </div>
                </form>
                </div>
        </div>
</div>
