README.txt
==========
The purpose of this module is to secure drupal backend by overriding admin path.
It's will rename path like '/admin/...' to '/something/...' or path '/user/..' 
to '/something else/..'. This module can be effective against registration spam 
bots or malicious people.
It's just implements hook_outbound_alter and hook_inbound_alter to rename paths.


Installation
------------
1. Install this module like any other Drupal module (place it in the
modules directory for your site and enable it on the `admin/build/modules` page.
2. Go to admin/config/user-interface/rename-admin-paths and configure how to 
override admin paths.
3. Clear your cache

Drupal 6
--------
If you are using Drupal 6, you can get the same feature by using the following 
code in your settings.php file.

function custom_url_rewrite_outbound(&$path, &$options, $original_path) {
    if (preg_match('|^admin(/{0,1}.*)|', $path, $matches)) {
        $path = 'backend'. $matches[1];
    }
}

function custom_url_rewrite_inbound(&$result, $path, $path_language) {
    if (preg_match('|^backend(/{0,1}.*)|', $path, $matches)) {
        $result = 'admin'. $matches[1];
    }
    if (preg_match('|^admin(/{0,1}.*)|', $path, $matches)) {
        $result = '404'. $matches[1];
    }
}
