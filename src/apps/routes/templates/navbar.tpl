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
<%
        /*
        <?php foreach( $nav_menu as $menu_id => $menu_item ) { ?>
                <li>
                <a href="<?php echo $menu_item["link"] ?>"
                <?php if ( $current_page == $menu_id ) echo ' class="active"'; ?>
                >
                <?php echo $menu_item["name"]; ?>
                        </a>
                </li>
                <?php } ?>
         </ul>
*/
%>
                 <ul class="nav navbar-nav navbar-right">
                <% if ( user.isLoggedIn() ) { %>
                        <li class="dropdown">
                                <a href="#" class="dropdown-toggle" data-toggle="dropdown">
                                        <span class="glyphicon glyphicon-user"></span>
                                        <strong><%= user.name() %></strong>
                    <span class="glyphicon glyphicon-chevron-down"></span>
                        </a>
                        <ul class="dropdown-menu" role="menu">
                        <li>
                                <a href="#" id="logout">Logout</a>
                        </li>
                </ul>
                        </li>
                    <% } else { %>
                    <li><a href="#login-modal" data-toggle="modal">Login</a></li>
                <% } %>

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
                <h3 class="modal-title">Login</h3>
            </div>
            <div class="modal-body row">
                <div class="col-md-12">
                    <form class="form-horizontal" id="login-form" data-toggle="validator" role="form" >
                        <div class="form-group">
                            <div class="alert hidden"></div>
                        </div>

                        <div class="form-group" >
                            <label class="control-label col-sm-2" for="username">Username<span class="text-danger">*</span></label>
                            <div class="input-group col-sm-10" >
                                <span class="input-group-addon"><i class="glyphicon glyphicon-user"></i></span>
                                <input type="text" class="form-control" name="username"  placeholder="Enter user name" required>
                            </div>
                            <div class="help-block with-errors"></div>
                        </div>
                        <div class="form-group" >
                            <label class="control-label col-sm-2" for="password">Password <span class="text-danger">*</span></label>
                            <div class="input-group col-sm-10">
                                <span class="input-group-addon"><i class="glyphicon glyphicon-lock"></i></span>
                                <input type="password" id="password" class="form-control" name="password" placeholder="Password" required>
                            </div>
                            <div class="help-block with-errors"></div>
                        </div>
                        <div class="form-group">
                            <div class="col-sm-offset-2 col-sm-10">
                              <div class="checkbox">
                                <label><input type="checkbox" name="remember-me"> Remember me</label>
                              </div>
                            </div>
                          </div>

                        <!--
                                <div class="form-group">
                                        <a href='user/password/reset/'>Forgot password?</a>
                                </div>
                                -->
                                        <div class="form-group text-center">
                                                  <button class="btn btn-primary type="submit">Submit</button>
                                        </div>
                               </form>
                             </div>
<!--
                                <div class="modal-footer">
                                    <a href="user/signup/" class="text-center register">Create an account</a>
                            </div>
                            -->
                        </div>
                </div>
        </div>
</div>
