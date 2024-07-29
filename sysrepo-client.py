from ncclient import manager
from lxml import etree

# Replace with your Netopeer server details
host = '192.168.202.16'
port = 830
username = 'root'
password = '12345'

def create_network_interface(interface_name):
    rpc_request = f'''
    <create-network-interface xmlns="http://example.com/aselsan-network-settings">
        <interface-name>{interface_name}</interface-name>
    </create-network-interface>
    '''

    with manager.connect(
        host=host,
        port=port,
        username=username,
        password=password,
        hostkey_verify=False
    ) as m:
        try:
            response = m.dispatch(etree.fromstring(rpc_request))
            print("RPC response:", response)
        except Exception as e:
            print(f"Error sending RPC: {str(e)}")

def set_ip_settings(interface_name, ip_address):
    rpc_request = f'''
    <set-ip-settings xmlns="http://example.com/aselsan-network-settings">
        <interface-name>{interface_name}</interface-name>
        <ip-address>{ip_address}</ip-address>
    </set-ip-settings>
    '''
    with manager.connect(
        host=host,
        port=port,
        username=username,
        password=password,
        hostkey_verify=False
    ) as m:
        try:
            response = m.dispatch(etree.fromstring(rpc_request))
            print("RPC response:", response)
        except Exception as e:
            print(f"Error sending RPC: {str(e)}")

def get_network_interfaces():
    rpc_request = '''
    <get-network-interfaces xmlns="http://example.com/aselsan-network-settings"/>
    '''

    with manager.connect(
        host=host,
        port=port,
        username=username,
        password=password,
        hostkey_verify=False
    ) as m:
        try:
            response = m.dispatch(etree.fromstring(rpc_request))
            print("RPC response:", response)
        except Exception as e:
            print(f"Error sending RPC: {str(e)}")

def delete_ip_address(interface_name, ip_address):
    rpc_request = f'''
    <delete-ip-address xmlns="http://example.com/aselsan-network-settings">
        <interface-name>{interface_name}</interface-name>
        <ip-address>{ip_address}</ip-address>
    </delete-ip-address>
    '''

    with manager.connect(
        host=host,
        port=port,
        username=username,
        password=password,
        hostkey_verify=False
    ) as m:
        try:
            response = m.dispatch(etree.fromstring(rpc_request))
            print("RPC response:", response)
        except Exception as e:
            print(f"Error sending RPC: {str(e)}")

if __name__ == "__main__":
    # Example usage:
    #create_network_interface("hasanaaaa")
    #set_ip_settings("hasanaaaa", "8.8.8.71")
    #get_network_interfaces()
    delete_ip_address("hasanaaaa", "8.8.8.71")
