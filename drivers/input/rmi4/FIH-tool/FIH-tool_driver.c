
#include <linux/input/FIH-tool/FIH-tool.h>

#include <linux/input/FIH-tool/tool_functions.h>

// ==========================================================================================================================

int get_rmi_memory( struct device*, void* );

int load_command( struct command_info*, struct load_command_data*, unsigned int );

void release_command_resource( struct command_info* );

struct touch_tool_data* allocate_memory( void );

void release_memory( struct touch_tool_data* );

unsigned int get_load_command_counter( void );

struct load_command_data* get_load_command_pointer( void );

void info_print( int, char*, ... );

void log_print( int, char*, ... );

void get_touch_ic_info( struct touch_tool_data*, void ( * )( int, char*, ... ) );

ssize_t tool_command( void*, struct device_attribute*, char* );

ssize_t tool_control( void*, struct device_attribute*, const char *, size_t );

ssize_t tool_info( void*, struct device_attribute*, char* );

// ==========================================================================================================================

static int __devinit	touch_tool_probe( struct platform_device *pdev )
{

	struct touch_tool_data	*tool_data;

	struct rmi_device	*device_data = NULL;

	rmi_get_bus_data( get_rmi_memory, ( void* )&device_data );

	if( !device_data )
	{

		printk( "TTUCH : Get RMI bus failed\n" );

		return	-ENOMEM;

	}

	if( !( tool_data = allocate_memory() ) )
		return	-ENOMEM;

	tool_data->rmi_dev = device_data;

	if( load_command( &tool_data->command, get_load_command_pointer(), get_load_command_counter() ) )
	{

		struct control_node_load	file_node;

		// Create file node in sys/class/touch/rmi4
		file_node.class_name = "touch", file_node.device_name = "rmi4";

		file_node.file_node_data = tool_data, file_node.control_read = tool_command, file_node.control_write = tool_control, file_node.info_read = tool_info;

		tool_data->file_node_class	= control_file_node_register( &file_node );

		dev_set_drvdata( &pdev->dev, tool_data );

		info_print( INFO_BUFFER_INIT, NULL, tool_data->info_buffer, INFO_BUFFER_SIZE );

		get_touch_ic_info( tool_data, log_print );
 
		printk( "TTUCH : Touch tool driver is ready\n" );

		return	0;

	}

	release_memory( tool_data );

	return	-EINVAL;

}

static int __devexit	touch_tool_remove( struct platform_device *pdev )
{

	struct touch_tool_data	*tool_data = dev_get_drvdata( &pdev->dev );

	control_file_node_unregister( tool_data->file_node_class );

	release_command_resource( &tool_data->command );

	release_memory( tool_data );

	return	0;

}

// ==========================================================================================================================

static struct platform_driver	touch_tool_device_driver =
{
	.probe		= touch_tool_probe,
	.remove		= __devexit_p( touch_tool_remove ),
	.driver		=
	{
		.name	= "fih_touch_tool",
		.owner	= THIS_MODULE,
	}
};

static int __init	fih_touch_tool_init( void )
{

	return	platform_driver_register( &touch_tool_device_driver );

}

static void __exit	fih_touch_tool_exit( void )
{

	platform_driver_unregister( &touch_tool_device_driver );

}

// ==========================================================================================================================

late_initcall( fih_touch_tool_init );

module_exit( fih_touch_tool_exit );

MODULE_AUTHOR( "Y.S Chang <yschang@fih-foxconn.com>" );

MODULE_DESCRIPTION( "FIH vibrator driver" );

MODULE_LICENSE( "GPL v2" );

MODULE_ALIAS( "platform:vibrator" );
