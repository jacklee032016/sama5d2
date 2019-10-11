<?php
namespace App\Base;

class File{
    
    public static function getSubfoldersFiles($folder, &$files)
    {
        $dh = new \DirectoryIterator($folder);
        // Dirctary object
        foreach ($dh as $item)
        {
            if (!$item->isDot() && strpos($item->getFilename(), '.Trash') === false)
            {
                if ($item->isDir())
                {
                    self::getSubfoldersFiles("$folder/$item", $files);
                } else {
                    $files[] = $folder . "/" . $item->getFilename();
                }
            }
        }
    }
}
